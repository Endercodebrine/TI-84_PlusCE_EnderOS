# ----------------------------
# Makefile Options
# ----------------------------

NAME = EnderOS
ICON = EnderOS.png
DESCRIPTION = "A new and improved interface for the TI-84"
COMPRESSED = NO

CFLAGS = -std=c23 -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
