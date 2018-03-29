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

# usage

1. give your imp to this optimizer like these (todo. support UDP), it will plus 1 imp for you.

    ```
    # need an ID for ecpm optimization.
    curl "http://127.0.0.1:8888/?id=123"
    ```

2. give your ecpm conversation cost into optimizer. like these

    ```
    # this conv cost 300 yen for id:123.
    curl "http://127.0.0.1:8888/?id=123&cost=300"
    ```

3. query the summary ecpm report by http interface

    ```
    curl "http://127.0.0.1:8888/?cmd=ranking"
    ```

# author

itsusony (meng.xiangliang1985@gmail.com)
