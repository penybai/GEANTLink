#
#    Copyright 1991-2020 Amebis
#    Copyright 2016 GÉANT
#
#    This file is part of GÉANTLink.
#
#    GÉANTLink is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    GÉANTLink is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with GÉANTLink. If not, see <http://www.gnu.org/licenses/>.
#

CleanSetup ::
	-if exist "$(OUTPUT_DIR)\$(LANG).$(PLAT).$(CFG).msi" del /f /q "$(OUTPUT_DIR)\$(LANG).$(PLAT).$(CFG).msi"
	-if exist "$(OUTPUT_DIR)\$(LANG).$(PLAT).$(CFG).mst" del /f /q "$(OUTPUT_DIR)\$(LANG).$(PLAT).$(CFG).mst"

"$(OUTPUT_DIR)\$(LANG).$(PLAT).$(CFG).2.msi" ::
	cd "MSI\Base"
	$(MAKE) /f "Makefile" /$(MAKEFLAGS) LANG=$(LANG) PLAT=$(PLAT) CFG=$(CFG)
	cd "$(MAKEDIR)"

"$(OUTPUT_DIR)\$(LANG).$(PLAT).$(CFG).msi" : \
	"$(OUTPUT_DIR)\$(LANG).$(PLAT).$(CFG).2.msi" \
	"$(OUTPUT_DIR)\$(PLAT).$(CFG).inf"
	-if exist $@ del /f /q $@
	copy /y "$(OUTPUT_DIR)\$(LANG).$(PLAT).$(CFG).2.msi" "$(@:"=).tmp" > NUL
	cscript.exe "MSI\MSIBuild\MSI.wsf" //Job:SetCAB //Nologo "$(@:"=).tmp" "$(OUTPUT_DIR)\$(PLAT).$(CFG).inf"
	msiinfo.exe "$(@:"=).tmp" /nologo /U 4
	move /y "$(@:"=).tmp" $@ > NUL

"$(OUTPUT_DIR)\$(LANG).$(PLAT).$(CFG).mst" : "$(OUTPUT_DIR)\en_US.$(PLAT).$(CFG).msi" "$(OUTPUT_DIR)\$(LANG).$(PLAT).$(CFG).msi"
	cscript.exe "MSI\MSIBuild\MSI.wsf" //Job:MakeMST //Nologo "$(OUTPUT_DIR)\en_US.$(PLAT).$(CFG).msi" "$(OUTPUT_DIR)\$(LANG).$(PLAT).$(CFG).msi" $@
