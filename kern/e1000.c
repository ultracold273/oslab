#include <inc/stdio.h>
#include <inc/error.h>

#include <kern/e1000.h>
#include <kern/pci.h>
#include <kern/pmap.h>

volatile uint32_t *e1000_regs_base;

// The transmit descriptor
__attribute__((__aligned__(PARA_BOUND)))
struct e1000_tx_desc tx_desc[NTXDESC];

// The transmit buffer
char tx_buf[NTXDESC][TXBUFSIZE];

// The receive descriptor
__attribute__((__aligned__(PARA_BOUND)))
struct e1000_rx_desc rx_desc[NRXDESC];

// The receive buffer 
char rx_buf[NRXDESC][RXBUFSIZE];

static void e1000_set(uint32_t off, uint32_t bit) {
    e1000_regs_base[off] |= bit;
}

static void e1000_clr(uint32_t off, uint32_t bit) {
    e1000_regs_base[off] &= ~bit;
}

static uint32_t e1000_read(uint32_t off) {
    return e1000_regs_base[off];
}

static void e1000_write(uint32_t off, uint32_t val) {
    e1000_regs_base[off] = val;
}

static void e1000_write_mask(uint32_t off, uint32_t mask, uint32_t val) {
    e1000_regs_base[off] &= ~mask;
    e1000_regs_base[off] |= val;
}

int e1000_send(void * srcaddr, int size) {
    char * baddr;
    struct e1000_tx_desc *tail = (struct e1000_tx_desc *) KADDR(e1000_read(E1000_TDT));
    struct e1000_tx_desc *tail_next = ((tail - tx_desc) < NTXDESC) ? tail + 1 : tx_desc;
    assert(size < TXBUFSIZE);
    if (tail_next->upper.fields.status & E1000_TXD_STAT_DD) {
        baddr = (char *) KADDR(tail_next->buffer_addr);
        memmove(baddr, srcaddr, size);
        tail_next->lower.flags.length = size;
        e1000_write(E1000_TDT, PADDR(tail_next));
    } else {
        return -E_NO_MEM;
    }
    return size;
}

static void e1000_trans_init() {
    int i;
    // Init transmit descriptor base address
    e1000_write(E1000_TDBAL, PADDR(tx_desc));
    // Init transmit descriptor total size
    e1000_write(E1000_TDLEN, NTXDESC * sizeof(struct e1000_tx_desc));
    // Init each tx_desc
    for (i = 0;i < NTXDESC;i++) {
        tx_desc[i].buffer_addr = PADDR(tx_buf[i]);
        tx_desc[i].lower.data |= E1000_TXD_CMD_RS;
        tx_desc[i].upper.data |= E1000_TXD_STAT_DD;
    }
    // Init Transmit descriptor head and tail
    e1000_write(E1000_TDH, 0);
    e1000_write(E1000_TDT, 0);
    // Init Control register
    e1000_set(E1000_TCTL, E1000_TCTL_EN | E1000_TCTL_PSP);
    e1000_write_mask(E1000_TCTL, E1000_TCTL_COLD, 0x40 << E1000_TCTL_COLD_SHIFT);
    e1000_write(E1000_TIPG, (12 << 20)|(8 << 10)| 10);
}

static void e1000_recv_init() {

}

// LAB 6: Your driver code here
int e1000_pci_attach (struct pci_func *pcif) {
    pci_func_enable(pcif);
    e1000_regs_base = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
    e1000_trans_init();
    e1000_recv_init();
    return 1;
}