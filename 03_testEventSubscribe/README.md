
## rpi4 
```bash 
ip addr add 192.168.1.101/24 dev eth0
ip route add 224.244.224.245 dev eth0 
```

## pc 
```bash
# set ip addr in pc
ip addr add 192.168.1.100/24 dev enp3s0
sudo ip route add 224.244.224.245 dev enp3s0 
```

## pc & rpi 
```bash
export VSOMEIP_CONFIGURATION= full path to configration file
export VSOMEIP_APPLICATION_NAME="name of the application in the json file"
```

## pc.json for server 
```json
{
    "unicast": "192.168.1.100",
    "logging": {
        "level": "debug",
        "console": true
    },
    "applications": [
        {
            "name": "hello_world_service",
            "id": "0x4444"
        }
    ],
    "services": [
        {
            "service": "0x1111",
            "instance": "0x2222",
            "methods": ["0x3333"],
            "unreliable": 30491
        }
    ],
    "service-discovery": {
        "enable": true,
        "multicast": "224.244.224.245",
        "port": 30490,
        "protocol": "udp",
        "initial_delay_min": 100,
        "initial_delay_max": 300,
        "repetition_base_delay": 200,
        "repetition_max": 3,
        "ttl": 3,
        "cyclic_offer_delay" : "2000",
        "request_response_delay" : "1500"
    },
    "routing": "hello_world_service"
}
```

## cient for rpi
```json
{
    "unicast":  "192.168.1.101",
    "logging": {
        "level": "debug",
        "console": true
    },
    "application" : 
    [
        {
            "name" : "hello_world_client",
            "id": "0x5555"
        }
    ],
    "service-discovery": {
        "enable": true,
        "multicast": "224.244.224.245",
        "port": 30490,
        "protocol": "udp",
        "initial_delay_min": 100,
        "initial_delay_max": 300,
        "repetition_base_delay": 200,
        "repetition_max": 3,
        "ttl": 3
    },
    "routing": "hello_world_client"
}
```

## rpi server.json
```json
{
    "unicast": "192.168.1.101",
    "logging": {
        "level": "debug",
        "console": true
    },
    "applications": [
        {
            "name": "hello_world_server",
            "id": "0x4444"
        }
    ],
    "services": [
        {
            "service": "0x1111",
            "instance": "0x2222",
            "unreliable": 30491
        }
    ],
    "service-discovery": {
        "enable": true,
        "multicast": "224.244.224.245",
        "port": 30490,
        "protocol": "udp",
        "initial_delay_min": 100,
        "initial_delay_max": 300,
        "repetition_base_delay": 200,
        "repetition_max": 3,
        "ttl": 3,
        "cyclic_offer_delay" : "2000",
        "request_response_delay" : "1500"
    }, 
    "routing": "hello_world_server"
}
```

## pc client.json
```json
{
    "unicast": "192.168.1.100",
    "logging": {
        "level": "debug",
        "console": true
    },
    "applications": [
        {
            "name": "hello_world_client",
            "id": "0x5555"
        }
    ],
    "service-discovery": {
        "enable": true,
        "multicast": "224.244.224.245",
        "port": 30490,
        "protocol": "udp",
        "initial_delay_min": 100,
        "initial_delay_max": 300,
        "repetition_base_delay": 200,
        "repetition_max": 3,
        "ttl": 3
    },
    "routing": "hello_world_client"
}
```