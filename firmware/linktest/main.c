/* Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sysincludes.h"

#include "cgptlib.h"
#include "load_kernel_fw.h"
#include "rollback_index.h"
#include "tlcl.h"
#include "vboot_common.h"
#include "vboot_kernel.h"
#include "vboot_nvstorage.h"


int main(void)
{
	/* cgptlib.h */
	GptInit(0);
	GptNextKernelEntry(0, 0, 0);
	GptUpdateKernelEntry(0, 0);

	/* load_kernel_fw.h */
	LoadKernel(0, 0);

	/* rollback_index.h */
	RollbackKernelRead(0);
	RollbackKernelWrite(0);
	RollbackKernelLock(0);

	/* tlcl.h */
	TlclStartup();
	TlclResume();
	TlclSelfTestFull();
	TlclContinueSelfTest();
	TlclDefineSpace(0, 0, 0);
	TlclWrite(0, 0, 0);
	TlclRead(0, 0, 0);
	TlclWriteLock(0);
	TlclReadLock(0);
	TlclIsOwned();
	TlclForceClear();
	TlclSetEnable();
	TlclSetDeactivated(0);
	TlclGetFlags(0, 0, 0);
	TlclExtend(0, 0, 0);
	TlclGetPermissions(0, 0);
#ifndef TPM2_MODE
	TlclAssertPhysicalPresence();
	TlclSetNvLocked();
	TlclClearEnable();
	TlclSetGlobalLock();
#endif

	/* vboot_api.h - entry points INTO vboot_reference */
	VbSelectAndLoadKernel(0, 0);

	/* vboot_common.h */
	OffsetOf(0, 0);
	GetPublicKeyData(0);
	GetPublicKeyDataC(0);
	GetSignatureData(0);
	GetSignatureDataC(0);
	VerifyMemberInside(0, 0, 0, 0, 0, 0);
	VerifyPublicKeyInside(0, 0, 0);
	VerifySignatureInside(0, 0, 0);
	PublicKeyInit(0, 0, 0);
	PublicKeyCopy(0, 0);
	PublicKeyToRSA(0);
	VerifyData(0, 0, 0, 0);
	VerifyDigest(0, 0, 0);
	KeyBlockVerify(0, 0, 0, 0);
	VerifyKernelPreamble(0, 0, 0);
	VbSharedDataInit(0, 0);
	VbSharedDataReserve(0, 0);
	VbSharedDataSetKernelKey(0, 0);

	VbNvSetup(0);
	VbNvGet(0, 0, 0);
	VbNvSet(0, 0, 0);
	VbNvTeardown(0);

	return 0;
}
