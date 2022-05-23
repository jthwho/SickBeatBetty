# CodeSign.cmake
#
# Contains utilities for code-signing files on Windows and macOS.
#
# Use code_sign_files() to sign files that already exist (usually used in a custom install script).
#
# NOTE: if you leave M_SIGN_PASSWORD empty, but an environment variable named "M_SIGN_PASSWORD" exists, the script
# will use the value in the environment variable as the password. This prevents any password info from being stored
# in build files.
#
# To use a certificate saved in a Windows certificate store, you have to have signtool.exe installed (from the Windows SDK),
# and you must set M_SIGN_CERT_NAME instead of M_SIGN_CERT_FILE. M_SIGN_CERT_NAME should be set to all or part of the
# subject name of the cert you wish to match, or its thumbprint.
#
# By default, the code will search for certificates in the current user's stores first. If no match is found there, the code
# will then search in machine global stores. To limit the search to machine global stores only (no user-specific ones), set
# M_SIGN_STORE_GLOBAL to TRUE.
#
# Functions:
# ----------
# code_sign_is_enabled(outname)
#   - Checks to see if code signing has been enabled, and the required certificate info has been set. Places result
#     in variable named [outname].
#
# code_sign_entitlements_enabled(outname)
#   - Checks to see if code signing entitlements have been specified. Places result in variable named [outname].
#
# code_sign_files([... full paths of files to sign ...])
#   - Immediately signs the given files. Files must already exist on disk when this is called, so this is usually
#     called as part of a custom install script.
#   - If code signing is disabled, this function is silently ignored.
#
# Common options:
# ---------------------------------------
# M_SIGN_DISABLE       - set to true to explicitly disable code-signing. If false, code-signing will be enabled if
#                        the required variables are set (M_SIGN_CERT_FILE or _NAME on Windows, or M_SIGN_CERT_NAME on macOS).
#
# M_SIGN_CERT_NAME     - common name (or enough of it to uniquely identify the signing certificate) (REQUIRED on macOS,
#                        REQUIRED on Windows if M_SIGN_CERT_FILE isn't set)
#
# Windows-only options:
# ---------------------------------------
# M_SIGN_CERT_STORE        - store to look up M_SIGN_CERT_NAME in, defaults to the "My" store if not provided.
# M_SIGN_CERT_STORE_GLOBAL - if TRUE, will only match certs in global stores. By default, matches user stores, then global stores.
# M_SIGN_CERT_FILE         - *.pfx file to read certification info from (REQUIRED on windows, if M_SIGN_CERT_NAME not set)
# M_SIGN_PASSWORD          - password used to access .pfx file.
# M_SIGN_DUAL              - dual-sign with SHA1 and SHA256. (Default: only sign with SHA256)
# M_SIGN_UAC_URL           - if provided, this URL will be displayed to user on UAC dialog (Default: no URL)
#
# On Windows, the name passed to M_SIGN_CERT_NAME should match all or part of the "Subject Name" field in the
# stored certificate.
#
#
# MacOS-only options:
# ---------------------------------------
# M_SIGN_KEYCHAIN      - keychain where cert is located (optional: defaults to login keychain, if not provided)
# M_SIGN_ENTITLEMENTS  - path to entitlements file (optional: default is no entitlements file)
#
# For details on using the entitlements file to enable app sandboxing, see here:
#   https://developer.apple.com/library/content/documentation/Miscellaneous/Reference/EntitlementKeyReference
#
# Note: Using an entitlements file will enable hardended run time. Hardended entitlements may be necessary.
#
# On MacOS, the name passed to M_SIGN_CERT_NAME should match all or part of the "Common Name" field in the certificate.
# Examples of certificate names:
#    "iPhone Distribution: Monetra Technologies, LLC."
#    "Mac Developer: John Smith" (Used for testing and development on personal machine)
#    "3rd Party Mac Developer Application: Monetra Technologies, LLC." (ONLY USE FOR APP STORE)
#    "Developer ID Application: Monetra Technologies, LLC." (USE FOR DIRECT DISTRIBUTION OUTSIDE THE APP STORE)
#
# If there is only one certificate loaded on your machine for each type of platform / deployment scenario, you
# can get away with specifying only the generic part (up to the colon). For example, if only one Mac distribution
# certificate is installed on the machine, setting M_SIGN_CERT_NAME to "Mac Developer Application" will find the
# proper key.
#
#
# Examples:
# ---------------------------------------
# Windows .pfx file, with no password:
#   $> cmake -DM_SIGN_CERT_FILE=C:\path\to\file.pfx
#
# Windows .pfx file, send password on command line (avoid, keeps copy of password in CMake build files and shell history):
#   $> cmake -DM_SIGN_CERT_FILE=C:\path\to\file.pfx -DM_SIGN_PASSWORD=pfx_file_password
#
# Windows .pfx file, password stored in environment variable named M_SIGN_PASSWORD (more secure than setting as a CMake variable):
#   $> cmake -DM_SIGN_CERT_FILE=C:\path\to\file.pfx
#
# Windows certificate installed in current user's default store, "My":
#   $> cmake -DM_SIGN_CERT_NAME="all or part of subject name"
#
# Windows certificate installed in local machine's default store, "My":
#   WARNING: all users on the system can use local machine stores
#   $> cmake -DM_SIGN_CERT_NAME="all or part of subject name" -DM_SIGN_CERT_STORE_GLOBAL=TRUE
#
# Windows certificate installed in a non-default current user store:
#   $> cmake -DM_SIGN_CERT_NAME="..." -DM_SIGN_CERT_STORE="Some Other Store"
#
# Windows certificate installed in a non-default local machine store:
#   WARNING: all users on the system can use local machine stores
#   $> cmake -DM_SIGN_CERT_NAME="..." -DM_SIGN_CERT_STORE="Some Other Store" -DM_SIGN_CERT_STORE_GLOBAL=TRUE
#

# Include guard.
if (_internal_codesign_already_included)
	return()
endif ()
set(_internal_codesign_already_included TRUE)

# Add empty cache entries for config options, if not set, so that user can see them in cmake-gui.
if (NOT DEFINED M_SIGN_DISABLE)
	option(M_SIGN_DISABLE "Force disable code signing?" FALSE)
endif ()
mark_as_advanced(FORCE M_SIGN_DISABLE)

if (WIN32)
	#include(CygwinPaths)

	# First, try to find osslsigntool or signtool.
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(arch x64)
	else ()
		set(arch x86)
	endif ()
	#
	#   -- First choice: SignTool from Windows 10.
	if (NOT SIGNTOOL)
		# Try to list all Windows 10 SDK versions, if any.
		set(win10_kit_versions)
		set(regkey "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots")
		set(regval "KitsRoot10")
		if (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
			# Note: must be a cache operation in order to read from the registry.
			get_filename_component(w10_kits_path "[${regkey};${regval}]" ABSOLUTE CACHE)
		else ()
			# On Cygwin, CMake's built-in registry query won't work. Use Cygwin utility "regtool" instead.
			execute_process(COMMAND regtool get "\\${regkey}\\${regval}"
				OUTPUT_VARIABLE w10_kits_path
				ERROR_QUIET
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)
			if (w10_kits_path)
				convert_windows_path(w10_kits_path)
			endif ()
		endif ()
		if (w10_kits_path)
			file(GLOB w10_kit_versions "${w10_kits_path}/bin/10.*")
			# Reverse list, so newer (higher-numbered) versions appear first.
			list(REVERSE w10_kit_versions)
		endif ()
		unset(w10_kits_path CACHE)
		if (w10_kit_versions)
			find_program(SIGNTOOL
					NAMES           signtool
					PATHS           ${w10_kit_versions}
					PATH_SUFFIXES   ${arch}
					                bin/${arch}
					                bin
					NO_DEFAULT_PATH
			)
		endif ()
	endif ()
	#
	#   -- Second choice: osslsigncode from Cygwin.
	#   -- The extra paths are for Windows CMD - when using Mingw from cygwin, will be on path already.
	find_program(OSSLSIGNCODE
		NAMES osslsigncode
		PATHS "C:/osslsigncode"
			  "$ENV{ProgramFiles}/osslsigncode"
		PATH_SUFFIXES bin
	)
	#
	#   -- Third choice: old copy of signtool (from Windows SDK 8.1 or earlier), or whatever random signtool is on the path.
	if (NOT SIGNTOOL AND NOT OSSLSIGNCODE)
		# When using VS, signtool.exe is guaranteed to be on the path. However, we want to try the registry checks first anyway,
		# to make sure we're using the newest signtool on the machine.
		find_program(SIGNTOOL
			NAMES signtool
			PATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot81]"
			      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot]"
			      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows;CurrentInstallFolder]"
			PATH_SUFFIXES ${arch}
			              bin/${arch}
			              bin
			NO_DEFAULT_PATH
		)
		if (NOT SIGNTOOL)
			# Only try searching the path if the registry keys above failed (because we need a NEW-ish signtool).
			find_program(SIGNTOOL signtool)
		endif ()

		if (NOT SIGNTOOL)
			message(FATAL_ERROR "Cannot sign code, could not find signtool or osslsigncode executables (need one of them)")
		endif ()
	endif ()

	# Once we've found our signing tool, validate the certificate source specified by the user.
	# Only signtool can be used with M_SIGN_CERT_NAME, because osslsigncode has no support for the windows certificate store.
	if (M_SIGN_CERT_FILE)
		get_filename_component(M_SIGN_CERT_FILE "${M_SIGN_CERT_FILE}" ABSOLUTE)
		if (NOT EXISTS "${M_SIGN_CERT_FILE}")
			message(FATAL_ERROR "Given certificate file \"${M_SIGN_CERT_FILE}\" does not exist (pass -DM_SIGN_CERT_FILE=... to change it).")
		endif ()
	elseif (M_SIGN_CERT_NAME)
		if (NOT SIGNTOOL)
			message(FATAL_ERROR "osslsigncode can't access the windows certificate store (pass -DM_SIGN_CERT_FILE=... instead of M_SIGN_CERT_NAME)")
		endif ()

		# If user selected a non-default cert store, verify that the store exists.
		string(TOLOWER "${M_SIGN_CERT_STORE}" store_lower)
		if (M_SIGN_CERT_STORE AND NOT store_lower STREQUAL "my")
			# If a cert store was specified, validate that it actually exists.
			set(cmd certutil -store)
			if (NOT M_SIGN_CERT_STORE_GLOBAL)
				list(APPEND cmd -user)
			endif ()

			execute_process(COMMAND ${cmd} "${M_SIGN_CERT_STORE}"
				RESULT_VARIABLE res
				OUTPUT_QUIET
				ERROR_QUIET
			)

			# If we tried the user store and it failed, try checking the global store.
			if ((NOT res EQUAL 0) AND (NOT M_SIGN_CERT_STORE_GLOBAL))
				# Set GLOBAL to TRUE, so that later code knows that we've fallen through to the global store.
				set(M_SIGN_CERT_STORE_GLOBAL TRUE)
				list(REMOVE_ITEM cmd "-user")
				execute_process(COMMAND ${cmd} "${M_SIGN_CERT_STORE}"
					RESULT_VARIABLE res
					OUTPUT_QUIET
					ERROR_QUIET
				)
			endif ()

			if (NOT res EQUAL 0)
				message(FATAL_ERROR "Selected store \"${M_SIGN_CERT_STORE}\" not found (pass -DM_SIGN_CERT_STORE=... to change it)")
			endif ()
		else ()
			set(M_SIGN_CERT_STORE "My")
		endif ()

		# Verify that the given cert name matches to a cert in the selected store.
		set(cmd certutil -store)
		if (NOT M_SIGN_CERT_STORE_GLOBAL)
			list(APPEND cmd -user)
		endif ()
		execute_process(COMMAND ${cmd} "${M_SIGN_CERT_STORE}" "*${M_SIGN_CERT_NAME}*"
			OUTPUT_QUIET
			ERROR_QUIET
			RESULT_VARIABLE res
		)
		# If we tried the user store and it failed, try checking the global store.
		if ((NOT res EQUAL 0) AND (NOT M_SIGN_CERT_STORE_GLOBAL))
			# Set GLOBAL to TRUE, so that later code knows that we've fallen through to the global store.
			set(M_SIGN_CERT_STORE_GLOBAL TRUE)
			list(REMOVE_ITEM cmd "-user")
			execute_process(COMMAND ${cmd} "${M_SIGN_CERT_STORE}" "*${M_SIGN_CERT_NAME}*"
				OUTPUT_QUIET
				ERROR_QUIET
				RESULT_VARIABLE res
			)
		endif ()

		if (NOT res EQUAL 0)
			message(FATAL_ERROR "No cert found in store \"${M_SIGN_CERT_STORE}\" that matched \
\"*${M_SIGN_CERT_NAME}*\" (pass -DM_SIGN_CERT_NAME=... to change it).")
		endif ()

	else ()
		set(M_SIGN_CERT_FILE "" CACHE FILEPATH "*.pfx file containing signing cert (required if M_SIGN_CERT_NAME not set)")
		if (SIGNTOOL)
			set(M_SIGN_CERT_NAME "" CACHE FILEPATH "name of cert in local cert store (required if M_SIGN_CERT_FILE not set)")
		endif ()
	endif ()

	if (NOT DEFINED M_SIGN_DUAL)
		option(M_SIGN_DUAL "Dual sign with SHA1 and SHA256?" FALSE)
		mark_as_advanced(FORCE M_SIGN_DUAL)
	endif ()

	# Take password from environment variable, if not already set by other means.
	if (NOT M_SIGN_PASSWORD)
		set(M_SIGN_PASSWORD "$ENV{M_SIGN_PASSWORD}")
	endif ()

	# Set up timestamp lists (both lists must be the same length).
	set(_internal_codesign_timestamp_list_sha1
		"http://timestamp.digicert.com"
		"http://timestamp.globalsign.com/scripts/timestamp.dll"
		"http://timestamp.comodoca.com"
		"http://tsa.starfieldtech.com"
		"http://timestamp.entrust.net/TSS/AuthenticodeTS"
	)
	set(_internal_codesign_timestamp_list_sha2
		"http://timestamp.digicert.com"
		"http://timestamp.globalsign.com/?signature=sha2"
		"http://timestamp.comodoca.com"
		"http://tsa.starfieldtech.com"
		"http://timestamp.entrust.net/TSS/RFC3161sha2TS"
	)
	list(LENGTH _internal_codesign_timestamp_list_sha2
		_internal_codesign_num_timestamps
	)

elseif (APPLE)
	# Do this on every run, to keep list of possible identities up-to-date.
	if (NOT DEFINED M_SIGN_CERT_NAME)
		set(M_SIGN_CERT_NAME "NONE")
	endif ()
	set(M_SIGN_CERT_NAME "${M_SIGN_CERT_NAME}" CACHE STRING "code signing identity (e.g., \"iPhone Developer\") (required for code signing)")
	# Get list of valid codesigning identities from system.
	execute_process(COMMAND security find-identity -v -p codesigning
		RESULT_VARIABLE res
		OUTPUT_VARIABLE lines
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	set(idents NONE)
	if (NOT M_SIGN_CERT_NAME STREQUAL "NONE")
		list(APPEND idents "${M_SIGN_CERT_NAME}")
	endif ()
	if (res EQUAL 0 AND lines)
		# Split string into list of lines.
		string(REGEX REPLACE ";" "\\\\;" lines "${lines}")
		string(REGEX REPLACE "\n" ";" lines "${lines}")
		# Parse signing cert identity from each line
		foreach(line ${lines})
			# Ex: 1) EE484E4BB4CE4779E5BF2AE3342636A035CC359 "iPhone Developer: Stephen Sorley (6JWJW49L4N)"
			if (line MATCHES "[0-9]+\\)[ \t]+[0-9a-fA-F]+[ \t]+\"(.+) \\([^ \t]+\\)\"")
				list(APPEND idents "${CMAKE_MATCH_1}")
			endif ()
		endforeach()
	endif ()
	# Populate drop-down box in cmake-gui with the list of valid codesigning identities.
	set_property(CACHE M_SIGN_CERT_NAME PROPERTY STRINGS "${idents}")
endif ()


function(code_sign_is_enabled out_enabled)
	set(enabled FALSE)
	set(quiet   FALSE)
	if ("QUIET" IN_LIST ARGN)
		set(quiet TRUE)
	endif ()

	if (M_SIGN_DISABLE AND (WIN32 OR APPLE))
		if (NOT quiet)
			message("Code signing manually disabled, use -DM_SIGN_DISABLE=FALSE to turn auto-enable back on")
		endif ()
	elseif (WIN32)
		if (M_SIGN_CERT_FILE OR (M_SIGN_CERT_NAME AND SIGNTOOL))
			set(enabled TRUE)
		elseif (NOT quiet)
			message("Code signing disabled, specify a *.pfx file with -DM_SIGN_CERT_FILE=... or a cert name from a store with -DM_SIGN_CERT_NAME=... to enable")
		endif ()
	elseif (APPLE)
		if (M_SIGN_CERT_NAME AND NOT M_SIGN_CERT_NAME STREQUAL "NONE")
			set(enabled TRUE)
		elseif (NOT quiet)
			message("Code signing disabled, specify certificate name from your keychain with -DM_SIGN_CERT_NAME=... to enable")
		endif ()
	endif ()

	set(${out_enabled} ${enabled} PARENT_SCOPE)
endfunction()


function(code_sign_entitlements_enabled out_enabled)
	set(enabled FALSE)
	if ("QUIET" IN_LIST ARGN)
		set(quiet TRUE)
	endif ()

	if (M_SIGN_ENTITLEMENTS)
		set(enabled TRUE)
	elseif (NOT quiet)
		message("Code signing entitlements disabled, specify -DM_SIGN_ENTITLEMENTS=... to enable")
	endif ()

	set(${out_enabled} ${enabled} PARENT_SCOPE)
endfunction()


# Switch to next timestamp server in the list.
# Helper for code_sign_files_win32
function(code_sign_next_ts_server)
	# Get current timestamp index.
	get_property(timestamp_idx GLOBAL PROPERTY _internal_codesign_timestamp_idx)
	if (NOT timestamp_idx MATCHES "^[0-9]+$")
		set(timestamp_idx 0)
	endif ()

	# Increment it. If we go past the end of the list, go back to first entry.
	math(EXPR timestamp_idx "${timestamp_idx} + 1")
	if (NOT timestamp_idx LESS _internal_codesign_num_timestamps)
		set(timestamp_idx 0)
	endif ()

	# Save back to global property list.
	set_property(GLOBAL PROPERTY _internal_codesign_timestamp_idx ${timestamp_idx})

	# Inform the user of the timeserver change.
	list(GET _internal_codesign_timestamp_list_sha2 ${timestamp_idx} timestamp_sha2)
	message("Changing timestamp server to: ${timestamp_sha2}")
endfunction()


# Helper for code_sign_files_win32
function(code_sign_get_cmd_win32 out_cmd1 out_cmd2 out_cmd_single out_using_signtool)
	set(${out_cmd} "" PARENT_SCOPE)

	# Get timestamps at current index.
	get_property(timestamp_idx GLOBAL PROPERTY _internal_codesign_timestamp_idx)
	if (NOT timestamp_idx MATCHES "^[0-9]+$")
		set(timestamp_idx 0)
	endif ()
	list(GET _internal_codesign_timestamp_list_sha1 ${timestamp_idx} timestamp_sha1)
	list(GET _internal_codesign_timestamp_list_sha2 ${timestamp_idx} timestamp_sha2)

	if (M_SIGN_CERT_FILE AND NOT EXISTS "${M_SIGN_CERT_FILE}")
		message(FATAL_ERROR "Cannot sign code, given PFX file in M_SIGN_CERT_FILE (\"${M_SIGN_CERT_FILE}\") does not exist")
	endif ()

	# Create the signing command we want to use, store in 'cmd'.
	if (SIGNTOOL)
		# See: https://successfulsoftware.net/2016/01/22/software-sha1-sha2-digital-certificates/
		#      https://stackoverflow.com/questions/26998439/signtool-with-certificate-stored-in-local-computer
		#
		# For .pfx files (M_SIGN_CERT_FILE):
		#   signtool sign /f <.pfx file> /p <password> /tr <timestamp URL> /fd sha256 /td sha256 [files ... (signed in-place)]
		#
		# For local cert store (M_SIGN_CERT_NAME):
		#   signtool sign [/sm] [/s <cert store name>] /n <cert name> /tr <timestamp URL> /fd sha256 /td sha256 [files ... (signed in-place)]
		#
		# Details on options:
		#     /tr: RFC 3161 timestamp server (current protocol)
		#     /t:  Authenticode timestamp server (old protocol)
		#     /fd: specify hash algo for file signatures (sha1, sha256 -> defaults to sha1)
		#     /td: specify hash algo for timestamps requested from RFC 3161 servers (sha1, sha256)
		#     /as: append to existing signatures, instead of replacing them
		#     /sm: if this flag present, will use global machine stores instead of user stores
		#     /s:  certificate store name, defaults to "My"
		#     /n:  all or part of the "subject name" of the certificate we want to use from the store

		# Common part of both commands.
		if (M_SIGN_CERT_FILE) # .pfx file
			# If we're using signtool from Cygwin, need to convert cygwin path to native Windows path.
			if (CMAKE_HOST_SYSTEM_NAME MATCHES "CYGWIN")
				convert_cygwin_path(M_SIGN_CERT_FILE)
			endif ()

			set(cmd "${SIGNTOOL}" sign /f "${M_SIGN_CERT_FILE}")
			if (M_SIGN_PASSWORD)
				list(APPEND cmd /p "${M_SIGN_PASSWORD}")
			endif ()
		else () # Windows certificate store
			# If using certificate store:
			set(cmd "${SIGNTOOL}" sign)
			if (M_SIGN_CERT_STORE_GLOBAL)
				list(APPEND cmd /sm)
			endif ()
			if (M_SIGN_CERT_STORE)
				list(APPEND cmd /s "${M_SIGN_CERT_STORE}")
			endif ()
			list(APPEND cmd /n "${M_SIGN_CERT_NAME}")
		endif ()

		# Dual-sign commands:
		# First command: SHA1 sign & timestamp for Windows 7 and older.
		set(cmd1 "${cmd}" /t "${timestamp_sha1}")

		# Second command: SHA256 sign & timestamp for Windows 8 and later (append as second signature).
		# Note: the "/as" flag (append signature) requires signtool.exe version 6.3 or later (Windows 8.1 SDK or newer).
		set(cmd2 "${cmd}" /tr "${timestamp_sha2}" /fd sha256 /td sha256 /as)

		# Single-sign command:
		set(cmd_single "${cmd}" /tr "${timestamp_sha2}" /fd sha256 /td sha256)

		set(using_signtool TRUE)
	else ()
		# See: http://www.elstensoftware.com/blog/2016/02/10/dual-signing-osslsigncode/
		#
		# osslsigncode -pkcs12 [.pfx file] -pass [password] -t [timestamp URL] -in [src file] -out [dest file]
		#   -h:    hash method (sha1, sha256, ...)
		#   -t:    Authenticode timestamp server (old protocol)
		#   -ts:   RFC 3161 timestamp server (current protocol)
		#   -nest: append to any existing signatures, instead of replacing them

		# Common part of both commands.
		set(cmd "${OSSLSIGNCODE}" -pkcs12 "${M_SIGN_CERT_FILE}")
		if (M_SIGN_PASSWORD)
			list(APPEND cmd -pass "${M_SIGN_PASSWORD}")
		endif ()

		# Dual-sign commands:
		# First command: SHA1 sign & timestamp for Windows 7 and older.
		set(cmd1 "${cmd}" -h sha1 -t "${timestamp_sha1}")

		# Second command: SHA256 sign & timestamp for Windows 8 and later (append as second signature).
		set(cmd2 "${cmd}" -nest -h sha256 -ts "${timestamp_sha2}")

		# Single-sign command:
		set(cmd_single "${cmd}" -h sha256 -ts "${timestamp_sha2}")

		set(using_signtool FALSE)
	endif ()

	set(${out_cmd1}           "${cmd1}"           PARENT_SCOPE)
	set(${out_cmd2}           "${cmd2}"           PARENT_SCOPE)
	set(${out_cmd_single}     "${cmd_single}"     PARENT_SCOPE)
	set(${out_using_signtool} "${using_signtool}" PARENT_SCOPE)
endfunction()


# Helper for code_sign_files_macos
function(code_sign_get_cmd_macos out_cmd)
	find_program(CODESIGN NAMES codesign)
	if (NOT CODESIGN)
		message(FATAL_ERROR "Cannot sign code, could not find 'codesign' executable")
	endif ()

	set(cmd "${CODESIGN}")

	if (M_SIGN_ENTITLEMENTS)
		# Enable hardened runtime
		list(APPEND cmd --options=runtime)

		list(APPEND cmd --entitlements "${M_SIGN_ENTITLEMENTS}")
	endif ()
   
	list(APPEND cmd -s "${M_SIGN_CERT_NAME}" --timestamp --signature-size=12000)

	if (M_SIGN_KEYCHAIN)
		list(APPEND cmd --keychain "${M_SIGN_KEYCHAIN}")
	endif ()

	set(${out_cmd} "${cmd}" PARENT_SCOPE)
endfunction()


# code_sign_files_win32([... paths of files to sign ...] [QUIET])
function(code_sign_files_win32)
	set(files "${ARGN}")

	set(quiet)
	if ("QUIET" IN_LIST files)
		set(quiet QUIET)
		list(REMOVE_ITEM files "QUIET")
	endif ()

	# Verify that signing is enabled. If it's not, return silently without doing anything.
	code_sign_is_enabled(enabled ${quiet})
	if (NOT enabled)
		return ()
	endif ()

	# Make command strings for single-sign and dual-sign.
	code_sign_get_cmd_win32(cmd1 cmd2 cmd_single using_signtool)

	foreach(path ${files})
		if (NOT EXISTS "${path}")
			message(FATAL_ERROR "Can't sign ${path}, no file exists at that path.")
		endif ()

		if (NOT quiet)
			message("  Signing: ${path}")
		endif ()

		if (NOT M_SIGN_DUAL OR (using_signtool AND path MATCHES "\.msi$"))
			# Note: can't dual-sign MSI files when using signtool (only osslsigncode can do that).
			set(cmds cmd_single)
		else ()
			# Dual sign.
			set(cmds cmd1 cmd2)
		endif ()

		# If this is a common executable file (.exe, .msi, .bat or .ps1), add a nice description that will
		# show up in the UAC dialog. If you don't do this, you'll see something like "27e1993a.msi" instead,
		# which looks kind of suspicious.
		get_filename_component(desc "${path}" NAME)
		if (desc MATCHES "(.[eE][xX][eE]|.[mM][sS][iI]|.[bB][aA][tT]|.[pP][sS]1)$")
			string(REGEX REPLACE "${CMAKE_MATCH_1}$" "" desc "${desc}")
		else ()
			set(desc)
		endif ()

		if (using_signtool)
			set(cmd_args "${path}")
			if (CMAKE_HOST_SYSTEM_NAME MATCHES "CYGWIN")
				# If we're using signtool from Cygwin, need to convert cygwin path to native Windows path.
				convert_cygwin_path(cmd_args)
			endif ()

			if (desc)
				if (M_SIGN_UAC_URL)
					set(cmd_args /du "${M_SIGN_UAC_URL}" "${cmd_args}")
				endif ()
				set(cmd_args /d "${desc}" "${cmd_args}")
			endif ()

			set(cmd_options)
		else ()
			# Note: annoyingly, osslsigncode won't let us use the same file for -in and -out (it causes a crash),
			#       so we have to write to a temp file and then manually overwrite the original file instead.
			get_filename_component(sign_dir  "${path}" DIRECTORY)
			get_filename_component(sign_file "${path}" NAME)
			set(cmd_args)
			if (desc)
				list(APPEND cmd_args -n "${desc}")
				if (M_SIGN_UAC_URL)
					list(APPEND cmd_args -i "${M_SIGN_UAC_URL}")
				endif ()
			endif ()
			list(APPEND cmd_args -in "${sign_file}" -out "${sign_file}.signed.tmp")

			set(cmd_options WORKING_DIRECTORY "${sign_dir}")
		endif ()

		foreach(cmdvar ${cmds})
			set(cmd "${${cmdvar}}")

			# May need to call sign command multiple times, if we hit the rate limit on a timestamp server.
			set(i 0)
			while(TRUE)
				execute_process(COMMAND ${cmd} ${cmd_args}
					RESULT_VARIABLE res
					ERROR_VARIABLE  err_output
					OUTPUT_VARIABLE err_output
					${cmd_options}
				)
				if (res EQUAL 0) # Success.
					break()
				elseif (i LESS "${_internal_codesign_num_timestamps}")
					message("Failed to reach timestamp server:\n${err_output}")
					message("Waiting 10 seconds before trying a different server...")
					execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 8 OUTPUT_QUIET ERROR_QUIET)
					# Change timestamp server.
					code_sign_next_ts_server()
					# Regenerate single-sign/dual-sign commands.
					code_sign_get_cmd_win32(cmd1 cmd2 cmd_single using_signtool)
					set(cmd "${${cmdvar}}")
				else ()
					string(REPLACE "${M_SIGN_PASSWORD}" "<password>" cmd_clean "${cmd}")
					message(FATAL_ERROR "Can't sign ${path}, command '${cmd_clean};${cmd_args}' failed:\n${err_output}")
				endif ()
				math(EXPR i "${i} + 1")
			endwhile()

			if (NOT using_signtool)
				# Bug fix: osslsigncode loses proper execute permissions. Need to copy them afterwards.
				if (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
					# Windows
					execute_process(COMMAND powershell -Command
						"& {&'Get-Acl' -Path ${sign_file} | &'Set-Acl' -Path ${sign_file}.signed.tmp}"
						WORKING_DIRECTORY "${sign_dir}"
					)
				else ()
					# Cygwin
					execute_process(COMMAND chmod "--reference=${sign_file}" "${sign_file}.signed.tmp"
						WORKING_DIRECTORY "${sign_dir}"
					)
				endif ()
				# Overwrite original exe with signed version from temp file.
				file(RENAME "${sign_dir}/${sign_file}.signed.tmp" "${sign_dir}/${sign_file}")
			endif ()
		endforeach ()
	endforeach()
endfunction()


# code_sign_files_macos([... paths of files to sign ...] [QUIET])
function(code_sign_files_macos)
	set(files "${ARGN}")

	set(quiet)
	if ("QUIET" IN_LIST files)
		set(quiet QUIET)
		list(REMOVE_ITEM files "QUIET")
	endif ()

	# Verify that signing is enabled. If it's not, return silently without doing anything.
	code_sign_is_enabled(enabled ${quiet})
	if (NOT enabled)
		return ()
	endif ()

	code_sign_get_cmd_macos(cmd)

	foreach(path ${files})
		if (NOT quiet)
			message("  Signing: ${path}")
		endif ()

		if (NOT EXISTS "${path}")
			message(FATAL_ERROR "Can't sign ${path}, no file exists at that path.")
		endif ()

		execute_process(COMMAND ${cmd} "${path}" RESULT_VARIABLE res)
		if (NOT res EQUAL 0)
			message(FATAL_ERROR "Can't sign ${path}, command '${cmd}' failed")
		endif ()
	endforeach()
endfunction()


# code_sign_files([... paths of files to sign ...] [QUIET])
function(code_sign_files)
	if (WIN32)
		code_sign_files_win32(${ARGN})
	elseif (APPLE)
		code_sign_files_macos(${ARGN})
	endif ()
endfunction()
