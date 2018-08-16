#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <net/ethernet.h>

int main(void)
{
	int sck;
	char          buf[1024];
        struct ifconf ifc;
        struct ifreq *ifr;
        int           nInterfaces;
        int           i;


	sck = socket(AF_INET, SOCK_DGRAM, 0);
        if(sck < 0)
        {
                perror("socket");
                return 1;
        }

        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = buf;
        if(ioctl(sck, SIOCGIFCONF, &ifc) < 0)
        {
                perror("ioctl(SIOCGIFCONF)");
                return 1;
        }

	ifr         = ifc.ifc_req;
        nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
        for(i = 0; i < nInterfaces; i++)
        {
                struct ifreq *item = &ifr[i];
		struct ether_addr *mac;
		char macbuf[255];

		if(ioctl(sck, SIOCGIFHWADDR, item) < 0)
                {
                        perror("ioctl(SIOCGIFHWADDR)");
                        return 1;
                }

		sprintf(macbuf,"%s", ether_ntoa((struct ether_addr *)item->ifr_hwaddr.sa_data));

		mac = (struct ether_addr *)item->ifr_hwaddr.sa_data;

		printf("\n dev = %s MAC = %s %02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n\n", item->ifr_name, macbuf, mac->ether_addr_octet[0],mac->ether_addr_octet[1],mac->ether_addr_octet[2],mac->ether_addr_octet[3],mac->ether_addr_octet[4],mac->ether_addr_octet[5]);
	}
	return 0;

}
