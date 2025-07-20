#
# Copyright (c) 2024, BlackBerry Limited. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# The basic QNX makefile definition
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

# A short description of the binary
define PINFO
PINFO DESCRIPTION=Camera example for buffer callbacks
endef

# The location to install the built binary on a target
INSTALLDIR = usr/bin

# Further QNX makefile definitions
include $(MKFILES_ROOT)/qmacros.mk
include $(MKFILES_ROOT)/qtargets.mk

# A space-separated list of libraries to be linked
LIBS += camapi
