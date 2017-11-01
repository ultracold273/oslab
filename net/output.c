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
		r = ipc_recv(&whom, &nsipcbuf, &perm);
		if (!(perm & PTE_P)) {
			continue;
		}
		while((r = sys_net_send(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len)) == -E_NO_MEM);
	}
}
