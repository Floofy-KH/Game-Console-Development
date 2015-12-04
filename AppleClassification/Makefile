# Subdirectories
DIRS := SPECode
# Target
PROGRAM_ppu := AppleClassification_prog 
# Include
INCLUDE := CImg.h FastEMD/emd_hat_signatures_interface.hpp
# Don't optimise code
#CC_OPT_LEVEL = -O0
CC_OPT_LEVEL = -g
# Imports
IMPORTS := SPECode/lib_ApplyThresholding.a -lspe2
# Local Defines
INSTALL_DIR = ~/AppleClassification/install/
INSTALL_FILES = $(PROGRAM_ppu)
# Imports
# buildutils/make.footer
include $(CELL_TOP)/buildutils/make.footer
