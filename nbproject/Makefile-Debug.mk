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
CND_PLATFORM=GNU-MacOSX
CND_DLIB_EXT=dylib
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/archive.o \
	${OBJECTDIR}/src/cJSON.o \
	${OBJECTDIR}/src/config.o \
	${OBJECTDIR}/src/hashmap.o \
	${OBJECTDIR}/src/kv_array.o \
	${OBJECTDIR}/src/main.o \
	${OBJECTDIR}/src/memwatch.o \
	${OBJECTDIR}/src/mmp.o \
	${OBJECTDIR}/src/part.o \
	${OBJECTDIR}/src/pathset.o \
	${OBJECTDIR}/src/response.o \
	${OBJECTDIR}/src/single_file.o \
	${OBJECTDIR}/src/textbuf.o \
	${OBJECTDIR}/src/url.o \
	${OBJECTDIR}/src/utils.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lm

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hrit.mmpupload

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hrit.mmpupload: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hrit.mmpupload ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/src/archive.o: src/archive.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/archive.o src/archive.c

${OBJECTDIR}/src/cJSON.o: src/cJSON.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/cJSON.o src/cJSON.c

${OBJECTDIR}/src/config.o: src/config.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/config.o src/config.c

${OBJECTDIR}/src/hashmap.o: src/hashmap.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/hashmap.o src/hashmap.c

${OBJECTDIR}/src/kv_array.o: src/kv_array.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/kv_array.o src/kv_array.c

${OBJECTDIR}/src/main.o: src/main.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/main.o src/main.c

${OBJECTDIR}/src/memwatch.o: src/memwatch.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/memwatch.o src/memwatch.c

${OBJECTDIR}/src/mmp.o: src/mmp.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/mmp.o src/mmp.c

${OBJECTDIR}/src/part.o: src/part.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/part.o src/part.c

${OBJECTDIR}/src/pathset.o: src/pathset.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pathset.o src/pathset.c

${OBJECTDIR}/src/response.o: src/response.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/response.o src/response.c

${OBJECTDIR}/src/single_file.o: src/single_file.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/single_file.o src/single_file.c

${OBJECTDIR}/src/textbuf.o: src/textbuf.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/textbuf.o src/textbuf.c

${OBJECTDIR}/src/url.o: src/url.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/url.o src/url.c

${OBJECTDIR}/src/utils.o: src/utils.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -DCOMMANDLINE -DMEMWATCH -DMEMWATCH_STDIO -Iinclude -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/utils.o src/utils.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hrit.mmpupload

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
