/*
 * %W% %E%
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * -Redistribution of source code must retain the above copyright notice, this
 *  list of conditions and the following disclaimer.
 * 
 * -Redistribution in binary form must reproduce the above copyright notice, 
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 * 
 * Neither the name of Oracle or the names of contributors may 
 * be used to endorse or promote products derived from this software without 
 * specific prior written permission.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL 
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING
 * ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN MICROSYSTEMS, INC. ("SUN")
 * AND ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE
 * AS A RESULT OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS
 * DERIVATIVES. IN NO EVENT WILL SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST 
 * REVENUE, PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, 
 * INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY 
 * OF LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, 
 * EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * 
 * You acknowledge that this software is not designed, licensed or intended
 * for use in the design, construction, operation or maintenance of any
 * nuclear facility.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jni.h"
#include "jvmti.h"

#include "agent_util.h"

/* Create major.minor.micro version string */
static void
version_check(jint cver, jint rver)
{
    jint cmajor, cminor, cmicro;
    jint rmajor, rminor, rmicro;

    cmajor = (cver & JVMTI_VERSION_MASK_MAJOR) >> JVMTI_VERSION_SHIFT_MAJOR;
    cminor = (cver & JVMTI_VERSION_MASK_MINOR) >> JVMTI_VERSION_SHIFT_MINOR;
    cmicro = (cver & JVMTI_VERSION_MASK_MICRO) >> JVMTI_VERSION_SHIFT_MICRO;
    rmajor = (rver & JVMTI_VERSION_MASK_MAJOR) >> JVMTI_VERSION_SHIFT_MAJOR;
    rminor = (rver & JVMTI_VERSION_MASK_MINOR) >> JVMTI_VERSION_SHIFT_MINOR;
    rmicro = (rver & JVMTI_VERSION_MASK_MICRO) >> JVMTI_VERSION_SHIFT_MICRO;
    stdout_message("Compile Time JVMTI Version: %d.%d.%d (0x%08x)\n", 
			cmajor, cminor, cmicro, cver);
    stdout_message("Run Time JVMTI Version: %d.%d.%d (0x%08x)\n", 
			rmajor, rminor, rmicro, rver);
    if ( (cmajor > rmajor) || (cmajor == rmajor && cminor > rminor) ) {
	fatal_error( 
	    "ERROR: Compile Time JVMTI and Run Time JVMTI are incompatible\n");
    }
}

/* Callback for JVMTI_EVENT_VM_INIT */
static void JNICALL 
vm_init(jvmtiEnv *jvmti, JNIEnv *env, jthread thread)
{
    jvmtiError err;
    jint       runtime_version;
   
    /* The exact JVMTI version doesn't have to match, however this
     *  code demonstrates how you can check that the JVMTI version seen 
     *  in the jvmti.h include file matches that being supplied at runtime
     *  by the VM. 
     */
    err = (*jvmti)->GetVersionNumber(jvmti, &runtime_version);
    check_jvmti_error(jvmti, err, "get version number");
    version_check(JVMTI_VERSION, runtime_version);
}

/* Agent_OnLoad() is called first, we prepare for a VM_INIT event here. */
JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
    jint                rc;
    jvmtiError          err;
    jvmtiEventCallbacks callbacks;
    jvmtiEnv           *jvmti;
    
    /* Get JVMTI environment */
    rc = (*vm)->GetEnv(vm, (void **)&jvmti, JVMTI_VERSION);
    if (rc != JNI_OK) {
	fatal_error("ERROR: Unable to create jvmtiEnv, GetEnv failed, error=%d\n", rc);
	return -1;
    }

    /* Set callbacks and enable event notifications */
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.VMInit                  = &vm_init;
    err = (*jvmti)->SetEventCallbacks(jvmti, &callbacks, sizeof(callbacks));
    check_jvmti_error(jvmti, err, "set event callbacks");
    err = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, 
			JVMTI_EVENT_VM_INIT, NULL);
    check_jvmti_error(jvmti, err, "set event notify");
    return 0;
}

/* Agent_OnUnload() is called last */
JNIEXPORT void JNICALL
Agent_OnUnload(JavaVM *vm)
{
}

