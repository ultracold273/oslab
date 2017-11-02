#include "ns.h"
#include <inc/error.h>

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	int r;
	envid_t whom;
	int perm;
	while(1) {
		// cprintf("Receiving.\n");
		r = ipc_recv(&whom, &nsipcbuf, &perm);
		// cprintf("Received.\n");
		if (!(perm & PTE_P)) {
			continue;
		}
		// cprintf("Here, datalen: %d\n", nsipcbuf.pkt.jp_len);
		// for(int i = 0;i < nsipcbuf.pkt.jp_len;i++) {
			// cprintf("0x%x ", nsipcbuf.pkt.jp_data[i]);
		// }
		while((r = sys_net_send(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len)) == -E_NO_MEM);
	}
}
