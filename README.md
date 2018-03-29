# ecpm_optimizer
an optimizer for ecpm's realtime evaluation

# architecture

![dataflow](https://raw.githubusercontent.com/itsusony/ecpm_optimizer/master/ecpm.png)

# how to build

1. need libevent2 (http://libevent.org/)
2. make

# TCP/IP turning

add these in your /etc/sysctl.conf if you use it in high traffic system.

```
net.ipv4.tcp_fin_timeout = 5
net.ipv4.tcp_syn_retries = 3
net.ipv4.tcp_synack_retries = 3
net.ipv4.tcp_rfc1337 = 1
net.core.somaxconn=65535
```

# author

itsusony (meng.xiangliang1985@gmail.com)
