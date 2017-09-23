/*
 * fontRedirect kernel plugin by cxziaho
 * Thanks to Rinnegatamante, Xerpi and devnoname120 for helping me in #henkaku
 *
 * MIT License
 * 
 * Copyright (c) 2017 cxz
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h> 
#include <psp2/io/fcntl.h>
#include <psp2/pvf.h>
#include <psp2/sysmodule.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <taihen.h>

#define BUILTIN_FONTS 17

#define FONT_DIR "ur0:"
#define FONT_NAME "font.otf"

// Check path exists
int exists(const char *path) {
	int fd = sceIoOpen(path, SCE_O_RDONLY, 0777);
	if (fd < 0)
		return 0;
	sceIoClose(fd);
	return 1;
}

static tai_hook_ref_t sysmod_load_ref;
static int sysmod_load_id;
static tai_hook_ref_t sysmod_unload_ref;
static int sysmod_unload_id;

static tai_hook_ref_t pvf_start_ref;
static int pvf_start_id;
static tai_hook_ref_t pvf_done_ref;
static int pvf_done_id;

static int pvf_path_injection;
static int pvf_font_injection[BUILTIN_FONTS];

ScePvfLibId scePvfNewLibHook(ScePvfInitRec *initParam, ScePvfError *errorCode) {
	// Prevent injecting data more than once
	if (pvf_path_injection != -1) goto DONE;
	
	int ret;
	// Load module info about SceLibPvf
	tai_module_info_t info;
	info.size = sizeof(info);
	ret = taiGetModuleInfo("SceLibPvf", &info);
	if (ret < 0)
		goto DONE;
	// Inject with new font path
	pvf_path_injection = taiInjectData(info.modid, 0, 0xE744, FONT_DIR, sizeof(FONT_DIR));
	// Inject 
	for (int i = 0; i < BUILTIN_FONTS; i++)
		pvf_font_injection[i] = taiInjectData(info.modid, 0, 0xE8A0+(0xD8*i), FONT_NAME, sizeof(FONT_NAME));
DONE:
	return TAI_CONTINUE(ScePvfLibId, pvf_start_ref, initParam, errorCode);
}

ScePvfError scePvfDoneLibHook(ScePvfLibId libID) {
	// Release the injections in SceLibPvf
	taiInjectRelease(pvf_path_injection);
	for (int i = 0; i < BUILTIN_FONTS; i++)
		taiInjectRelease(pvf_font_injection[i]);
	return TAI_CONTINUE(ScePvfError, pvf_done_ref, libID);
}

int hook_sysmod_load(SceSysmoduleInternalModuleId id, SceSize args, void *argp, void *unk) {
	int ret = TAI_CONTINUE(int, sysmod_load_ref, id, args, argp, unk);
	// Check if loaded sysmodule is SCE_SYSMODULE_INTERNAL_PAF
	if (ret >= 0 && id == SCE_SYSMODULE_INTERNAL_PAF) {
		// Hook scePvfNewLib in ScePaf
		pvf_start_id = taiHookFunctionImport(&pvf_start_ref,
											 "ScePaf",
											 0x68D59260,
											 0x72E58672,
											 scePvfNewLibHook);
		// Hook scePvfDoneLib in ScePaf
		pvf_done_id = taiHookFunctionImport(&pvf_done_ref,
											 "ScePaf",
											 0x68D59260,
											 0xE17717EC,
											 scePvfDoneLibHook);
	}
	return ret;
}

int hook_sysmod_unload(SceSysmoduleInternalModuleId id) {
	int ret = TAI_CONTINUE(int, sysmod_unload_ref, id);
	// Check if loaded sysmodule is SCE_SYSMODULE_INTERNAL_PAF
	if (ret >= 0 && id == SCE_SYSMODULE_INTERNAL_PAF) {
		// Release hooks in ScePaf
		taiHookRelease(pvf_start_id, pvf_start_ref);
		taiHookRelease(pvf_done_id, pvf_done_ref);
	}
	return ret;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
	pvf_path_injection = -1;
	
	if (exists(FONT_DIR FONT_NAME)) {
		// Hook import of sceSysmoduleLoadModuleInternalWithArg in SceShell
		sysmod_load_id = taiHookFunctionImport(&sysmod_load_ref,
											   "SceShell",
											   0x03FCF19D,
											   0xC3C26339,
											   hook_sysmod_load);
		// Hook import of sceSysmoduleUnloadModuleInternal in SceShell
		sysmod_unload_id = taiHookFunctionImport(&sysmod_unload_ref,
											   "SceShell",
											   0x03FCF19D,
											   0xFF206B19,
											   hook_sysmod_unload);
	}
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
	// Release hooks in SceShell
	taiHookRelease(sysmod_load_id, sysmod_load_ref);
	taiHookRelease(sysmod_unload_id, sysmod_unload_ref);
	return SCE_KERNEL_STOP_SUCCESS;
}
