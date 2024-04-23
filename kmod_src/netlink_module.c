#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/netlink.h>

#define NETLINK_TEST     30
#define USER_PORT        100
int netlink_count = 0;
char netlink_kmsg[30];

struct sock *nlsk = NULL;
extern struct net init_net;

int send_usrmsg(char *pbuf, uint16_t len)
{
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;

    int ret;

    //Create sk_buff using nlmsg_new().
    nl_skb = nlmsg_new(len, GFP_ATOMIC);
    if(!nl_skb)
    {
        printk("netlink alloc failure\n");
        return -1;
    }

    //Set up nlmsghdr.
    nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_TEST, len, 0);
    if(nlh == NULL)
    {
        printk("nlmsg_put failaure \n");
        nlmsg_free(nl_skb);  //If nlmsg_put() failed, nlmsg_free() will free sk_buff.
        return -1;
    }

    //Copy pbuf to nlmsghdr payload.
    memcpy(nlmsg_data(nlh), pbuf, len);
    ret = netlink_unicast(nlsk, nl_skb, USER_PORT, MSG_DONTWAIT);

    return ret;
}

static void netlink_rcv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    char *umsg = NULL;
    //char *kmsg = "hello users!!!";
    char *kmsg;

    if(skb->len >= nlmsg_total_size(0))
    {
        netlink_count++;
    snprintf(netlink_kmsg, sizeof(netlink_kmsg), "hello users count=%d", netlink_count);
    kmsg = netlink_kmsg;
        nlh = nlmsg_hdr(skb);  //Get nlmsghdr from sk_buff.
        umsg = NLMSG_DATA(nlh); //Get payload from nlmsghdr.
        if(umsg)
        {
            printk("kernel recv from user: %s\n", umsg);
            send_usrmsg(kmsg, strlen(kmsg));
        }
    }
}

struct netlink_kernel_cfg cfg = { 
        .input  = netlink_rcv_msg, /* set recv callback */
};  

__init int netlink_test_init(void)
{
    /* Create netlink socket */
    nlsk = (struct sock *)netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
    if(nlsk == NULL)
    {   
        printk("netlink_kernel_create error !\n");
        return -1; 
    }   
    printk("netlink_test_init\n");
    
    return 0;
}

__exit void netlink_test_exit(void)
{
    if (nlsk){
        netlink_kernel_release(nlsk); /* release ..*/
        nlsk = NULL;
    }   
    printk("netlink_test_exit!\n");
}

module_init(netlink_test_init);
module_exit(netlink_test_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("netlink module");