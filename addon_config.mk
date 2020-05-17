# All variables and this file are optional, if they are not present the PG and the
# makefiles will try to parse the correct values from the file system.
#
# Variables that specify exclusions can use % as a wildcard to specify that anything in
# that position will match. A partial path can also be specified to, for example, exclude
# a whole folder from the parsed paths from the file system
#
# Variables can be specified using = or +=
# = will clear the contents of that variable both specified from the file or the ones parsed
# from the file system
# += will add the values to the previous ones in the file or the ones parsed from the file
# system
#
# The PG can be used to detect errors in this file, just create a new project with this addon
# and the PG will write to the console the kind of error and in which line it is

meta:
	ADDON_NAME = ofxLaser
	ADDON_DESCRIPTION = Addon for controlling one or more ILDA lasers.
	ADDON_AUTHOR = Seb Lee-Delisle
	ADDON_TAGS = "laser" "ilda" "etherdream" "idn" "laserdock" "lasercube"
	ADDON_URL = http://github.com/sebleedelisle/ofxLaser

common:
	# dependencies with other addons, a list of them separated by spaces
	# or use += in several lines
	# NOTE - we only include ofxKinect as it's the easiest way to ensure libusb
	# is included cross platform
	ADDON_DEPENDENCIES = ofxXmlSettings ofxGui ofxOpenCv ofxNetwork ofxPoco ofxSvg ofxKinect
	
	# include search paths, this will be usually parsed from the file system
	# but if the addon or addon libraries need special search paths they can be
	# specified here separated by spaces or one per line using +=
	# ADDON_INCLUDES =

	# any special flag that should be passed to the compiler when using this
	# addon
	# ADDON_CFLAGS =

	# any special flag that should be passed to the linker when using this
	# addon, also used for system libraries with -lname
	# ADDON_LDFLAGS =

	# linux only, any library that should be included in the project using
	# pkg-config
	# ADDON_PKG_CONFIG_LIBRARIES = libusb-1.0

	# osx/iOS only, any framework that should be included in the project
	# ADDON_FRAMEWORKS =

	# source files, these will be usually parsed from the file system looking
	# in the src folders in libs and the root of the addon. if your addon needs
	# to include files in different places or a different set of files per platform
	# they can be specified here
	# ADDON_SOURCES =

	# some addons need resources to be copied to the bin/data folder of the project
	# specify here any files that need to be copied, you can use wildcards like * and ?
	# ADDON_DATA =

	# when parsing the file system looking for libraries exclude this for all or
	# a specific platform
	# ADDON_LIBS_EXCLUDE =

	linux64:
		# binary libraries, these will be usually parsed from the file system but some 
		# libraries need to passed to the linker in a specific order/
		# 
		# For example in the ofxOpenCV addon we do something like this:
		#
		# ADDON_LIBS =
		# ADDON_LIBS += libs/opencv/lib/linuxarmv6l/libopencv_legacy.a
		# ADDON_LIBS += libs/opencv/lib/linuxarmv6l/libopencv_calib3d.a
		# ...
		#ADDON_PKG_CONFIG_LIBRARIES = libusb-1.0
		#ADDON_INCLUDES_EXCLUDE = libs/libusb-1.0/%

	linux:
		#ADDON_PKG_CONFIG_LIBRARIES = libusb-1.0
		#ADDON_INCLUDES_EXCLUDE = libs/libusb-1.0/%


	win_cb:

	linuxarmv6l:
	linuxarmv7l:
	android/armeabi:	
	android/armeabi-v7a: