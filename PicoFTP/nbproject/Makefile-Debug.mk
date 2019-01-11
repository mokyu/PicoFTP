#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/client.o \
	${OBJECTDIR}/ftp.o \
	${OBJECTDIR}/lookup.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/path.o \
	${OBJECTDIR}/server.o \
	${OBJECTDIR}/worker.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f2

# Test Object Files
TESTOBJECTFILES= \
	${TESTDIR}/tests/c_path.o

# C Compiler Flags
CFLAGS=-pthread -g -O0 -ggdb3 -fstack-check -fstack-protector-all -fno-omit-frame-pointer -Wall -pedantic -lcunit -fvar-tracking -fvar-tracking-assignments

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=`pkg-config --libs cunit`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/picoftp

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/picoftp: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/picoftp ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/client.o: client.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g `pkg-config --cflags cunit`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/client.o client.c

${OBJECTDIR}/ftp.o: ftp.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g `pkg-config --cflags cunit`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ftp.o ftp.c

${OBJECTDIR}/lookup.o: lookup.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g `pkg-config --cflags cunit`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lookup.o lookup.c

${OBJECTDIR}/main.o: main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g `pkg-config --cflags cunit`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.c

${OBJECTDIR}/path.o: path.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g `pkg-config --cflags cunit`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/path.o path.c

${OBJECTDIR}/server.o: server.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g `pkg-config --cflags cunit`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/server.o server.c

${OBJECTDIR}/worker.o: worker.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g `pkg-config --cflags cunit`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/worker.o worker.c

# Subprojects
.build-subprojects:

# Build Test Targets
.build-tests-conf: .build-tests-subprojects .build-conf ${TESTFILES}
.build-tests-subprojects:

${TESTDIR}/TestFiles/f2: ${TESTDIR}/tests/c_path.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.c} -o ${TESTDIR}/TestFiles/f2 $^ ${LDLIBSOPTIONS}   -lcunit 


${TESTDIR}/tests/c_path.o: tests/c_path.c 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.c) -g `pkg-config --cflags cunit`   -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/c_path.o tests/c_path.c


${OBJECTDIR}/client_nomain.o: ${OBJECTDIR}/client.o client.c 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/client.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g `pkg-config --cflags cunit`   -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/client_nomain.o client.c;\
	else  \
	    ${CP} ${OBJECTDIR}/client.o ${OBJECTDIR}/client_nomain.o;\
	fi

${OBJECTDIR}/ftp_nomain.o: ${OBJECTDIR}/ftp.o ftp.c 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/ftp.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g `pkg-config --cflags cunit`   -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ftp_nomain.o ftp.c;\
	else  \
	    ${CP} ${OBJECTDIR}/ftp.o ${OBJECTDIR}/ftp_nomain.o;\
	fi

${OBJECTDIR}/lookup_nomain.o: ${OBJECTDIR}/lookup.o lookup.c 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/lookup.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g `pkg-config --cflags cunit`   -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lookup_nomain.o lookup.c;\
	else  \
	    ${CP} ${OBJECTDIR}/lookup.o ${OBJECTDIR}/lookup_nomain.o;\
	fi

${OBJECTDIR}/main_nomain.o: ${OBJECTDIR}/main.o main.c 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/main.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g `pkg-config --cflags cunit`   -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_nomain.o main.c;\
	else  \
	    ${CP} ${OBJECTDIR}/main.o ${OBJECTDIR}/main_nomain.o;\
	fi

${OBJECTDIR}/path_nomain.o: ${OBJECTDIR}/path.o path.c 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/path.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g `pkg-config --cflags cunit`   -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/path_nomain.o path.c;\
	else  \
	    ${CP} ${OBJECTDIR}/path.o ${OBJECTDIR}/path_nomain.o;\
	fi

${OBJECTDIR}/server_nomain.o: ${OBJECTDIR}/server.o server.c 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/server.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g `pkg-config --cflags cunit`   -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/server_nomain.o server.c;\
	else  \
	    ${CP} ${OBJECTDIR}/server.o ${OBJECTDIR}/server_nomain.o;\
	fi

${OBJECTDIR}/worker_nomain.o: ${OBJECTDIR}/worker.o worker.c 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/worker.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.c) -g `pkg-config --cflags cunit`   -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/worker_nomain.o worker.c;\
	else  \
	    ${CP} ${OBJECTDIR}/worker.o ${OBJECTDIR}/worker_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f2 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
