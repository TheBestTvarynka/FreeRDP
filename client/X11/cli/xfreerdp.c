/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * X11 Client
 *
 * Copyright 2011 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 * Copyright 2012 HP Development Company, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <freerdp/config.h>

#include <winpr/crt.h>
#include <winpr/synch.h>
#include <winpr/thread.h>

#include <freerdp/streamdump.h>
#include <freerdp/freerdp.h>
#include <freerdp/client/cmdline.h>

#include "../xf_client.h"
#include "../xfreerdp.h"

// #include <dlfcn.h>
// #include "tbt/pkcs11.h"

static void xfreerdp_print_help(void)
{
	printf("Keyboard Shortcuts:\n");
	printf("\t<Right CTRL>\n");
	printf("\t\treleases keyboard and mouse grab\n");
	printf("\t<CTRL>+<ALT>+<Return>\n");
	printf("\t\ttoggles fullscreen state of the application\n");
	printf("\t<CTRL>+<ALT>+c\n");
	printf("\t\ttoggles remote control in a remote assistance session\n");
	printf("\tAction Script\n");
	printf("\t\tExecutes a predefined script on key press.\n");
	printf("\t\tShould the script not exist it is ignored.\n");
	printf("\t\tScripts can be provided at the default localtion ~/.config/freerdp/action.sh or as "
	       "command line argument /action:script:<path>\n");
	printf("\t\tThe script will receive the current key combination as argument.\n");
	printf("\t\tThe output of the script is parsed for 'key-local' which tells that the script "
	       "used the key combination, otherwise the combination is forwarded to the remote.\n");
}

// int tbt() {
//     printf("TBT: TheBestTvarynka: 2.\n");

//     char* pkcs11_module = "libykcs11.so.2.5.2";
//     CK_ULONG nslots = 0;
//     CK_SLOT_ID slots[64];
//     CK_RV rv = 0;
//     void* module = NULL;
//     CK_FUNCTION_LIST_PTR p11;
    
//     typedef CK_RV (*c_get_function_list_t)(CK_FUNCTION_LIST_PTR_PTR);
//     c_get_function_list_t c_get_function_list = NULL;

//     module = dlopen(pkcs11_module, RTLD_LOCAL | RTLD_LAZY);
//     if (!module) {
//         printf("TBT: can not load pkcs11 module: %p. `%s`: %s :(\n", module, pkcs11_module, dlerror());
//         return 2;
//     } else {
//         printf("TBT: pkcs11 module successfully loaded!\n");
//     }

//     c_get_function_list = (c_get_function_list_t)dlsym(module, "C_GetFunctionList");
//     if (!c_get_function_list)
//     {
//         printf("TBT: can not find `C_GetFunctionList` symbol: %p :(\n", c_get_function_list);
//         return 2;
//     } else {
//         printf("TBT: successfully found C_GetFunctionList!\n");
//     }

//     rv = c_get_function_list(&p11);
//     if (rv != CKR_OK) {
//         printf("TBT: C_GetFunctionList: failed! :(\n");
//         return 2;
//     } else {
//         printf("TBT: C_GetFunctionList: succeeded;\n");
//     }

//     rv = p11->C_Initialize(NULL);
//     if (rv != CKR_OK) {
//         printf("TBT: C_Initialize: failed! :(\n");
//         // return 2;
//     } else {
//         printf("TBT: C_Initialize: succeeded;\n");
//     }

//     rv = p11->C_GetSlotList(CK_TRUE, NULL, &nslots);
//     if (rv != CKR_OK) {
//         printf("TBT: C_GetSlotList(true, null, ptr): failed! :(\n");
//         return 2;
//     } else {
//         printf("TBT: C_GetSlotList(true, null, ptr): succeeded;\n");
//         printf("TBT: C_GetSlotList(true, null, ptr): we have %ld | %lu slots;\n", nslots, nslots);
//     }

//     rv = p11->C_GetSlotList(CK_TRUE, slots, &nslots);
//     if (rv != CKR_OK) {
//         printf("TBT: C_GetSlotList(true, ptr, ptr): failed! :(\n");
//         return 2;
//     } else {
//         printf("TBT: C_GetSlotList(true, ptr, ptr): succeeded;\n");
//         printf("TBT: C_GetSlotList(true, null, ptr): we have %ld | %lu slots;\n", nslots, nslots);
//     }

//     return 0;
// }

int main(int argc, char* argv[])
{
	// printf("TBT: before: smartcard_enumerateCerts <- read int <- FreeRDp6:\n");
	// int myInt = 0;
	// tbt();
	// printf("TBT: before: smartcard_enumerateCerts <-\n");
	// return 0;

	int rc = 1;
	int status = 0;
	HANDLE thread = NULL;
	xfContext* xfc = NULL;
	DWORD dwExitCode = 0;
	rdpContext* context = NULL;
	rdpSettings* settings = NULL;
	RDP_CLIENT_ENTRY_POINTS clientEntryPoints = { 0 };

	clientEntryPoints.Size = sizeof(RDP_CLIENT_ENTRY_POINTS);
	clientEntryPoints.Version = RDP_CLIENT_INTERFACE_VERSION;

	RdpClientEntry(&clientEntryPoints);

	context = freerdp_client_context_new(&clientEntryPoints);
	if (!context)
		return 1;

	settings = context->settings;
	xfc = (xfContext*)context;

	status = freerdp_client_settings_parse_command_line(context->settings, argc, argv, FALSE);
	if (status)
	{
		rc = freerdp_client_settings_command_line_status_print(settings, status, argc, argv);

		xfreerdp_print_help();

		if (freerdp_settings_get_bool(settings, FreeRDP_ListMonitors))
			xf_list_monitors(xfc);

		goto out;
	}

	if (!stream_dump_register_handlers(context, CONNECTION_STATE_MCS_CREATE_REQUEST, FALSE))
		goto out;

	if (freerdp_client_start(context) != 0)
		goto out;

	thread = freerdp_client_get_thread(context);

	WaitForSingleObject(thread, INFINITE);
	GetExitCodeThread(thread, &dwExitCode);
	rc = xf_exit_code_from_disconnect_reason(dwExitCode);

	freerdp_client_stop(context);

out:
	freerdp_client_context_free(context);

	return rc;
}
