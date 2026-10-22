# FIT3143 Docker Setup on Windows

This guide prepares a FIT3143 container on Windows and follows the two-laptop eduroam workflow. The laptops may instead use any shared Wi-Fi/LAN connection, provided it permits direct traffic between peer devices. Use the same container architecture on every participating laptop; Windows groups normally use `linux/amd64`.

## 1. Architecture overview

| Layer | Role |
| --- | --- |
| Laptop A (`mpi-a`) | Launches MPI jobs. |
| Laptop B (`mpi-b`) | Runs its assigned MPI processes. |
| Docker container | Provides Open MPI, the working directory, and SSH. |
| Shared Wi-Fi/LAN | Carries traffic between the two laptops. |

## 2. Prerequisites

Complete this checklist on **both** PCs:

- [ ] Docker Desktop is installed and can run Linux containers.
- [ ] Both PCs are connected to the same peer-reachable Wi-Fi/LAN network.
- [ ] You know each laptop's IP address and have exchanged it with your partner.
- [ ] You have the course program file (for example, `task1.c`) available on Laptop A.
- [ ] You understand the supplied image uses the `student` account and `/home/student/mpi-cluster` as its working directory.

## 3. Enable virtualization, WSL, Hyper-V, and host networking

Enable hardware virtualization in BIOS/UEFI. Open **PowerShell as Administrator** and run:

```powershell
wsl --install
```

Restart if prompted. In **Turn Windows features on or off**, enable **Windows Subsystem for Linux**, **Virtual Machine Platform**, and **Hyper-V** when it is available.

Start Docker Desktop, select **Linux containers**, then open **Settings > Resources > Network**. Enable **Enable host networking** and select **Apply & restart**.

```powershell
docker version
docker info --format '{{.OSType}}'
```

The second command must print `linux`.

## 4. Find the eduroam or other peer-network address

```powershell
ipconfig
```

Record the IPv4 address under **Wireless LAN adapter Wi-Fi** as `LOCAL_IP`. Obtain the peer laptop's eduroam IP (or other reachable Wi-Fi/LAN IP) as `PEER_IP`, then test it:

```powershell
ping.exe <PEER_IP>
```

If ping fails, eduroam or the selected network is isolating client devices. Do not continue until the two hosts can reach one another. Wi-Fi IPs may change after reconnecting; if that happens, repeat this check and update the hostname-resolution entries.

## 5. Pull and start the course image

```powershell
docker pull monashfit/fit3143:latest
New-Item -ItemType Directory -Force "$HOME\mpi-cluster"
```

This is the Docker command from the supplied guide. On Laptop A use `mpi-a`; Laptop B uses the same command with `mpi-b`.

```powershell
docker run -d `
  --name fit3143 `
  --hostname mpi-a `
  --network host `
  --add-host "mpi-a:<LAPTOP_A_IP>" `
  --add-host "mpi-b:<LAPTOP_B_IP>" `
  --cap-add=NET_ADMIN `
  -v "$HOME\mpi-cluster:/home/student/mpi-cluster" `
  monashfit/fit3143:latest sleep infinity
```


## 6. Configure firewalls and install container tools

On each PC, open **PowerShell as Administrator** and allow inbound traffic from the peer laptop through Windows Defender Firewall. Replace `<PEER_IP>` with the other laptop's Wi-Fi/LAN address:

```powershell
New-NetFirewallRule -DisplayName 'FIT3143 MPI peer' -Direction Inbound -Action Allow -RemoteAddress <PEER_IP> -Profile Domain,Private,Public
Get-NetFirewallRule -DisplayName 'FIT3143 MPI peer'
```

Install UFW and the networking/SSH tools **inside each Docker container**. UFW is the container firewall; Windows Defender Firewall above is the host firewall. The `NET_ADMIN` capability in the `docker run` command is required for UFW to manage the container's firewall rules. Replace `<PEER_IP>` with the other laptop's address; this permits the SSH and dynamic MPI traffic originating from that peer.

```powershell
docker exec -u root fit3143 bash

apt-get update && apt-get install -y openssh-server iproute2 iputils-ping ufw && mkdir -p /var/run/sshd && printf "Port 2222\n" > /etc/ssh/sshd_config.d/mpi-lab.conf && ufw allow from <PEER_IP> && ufw --force enable && service ssh restart
```

Final command should show
```
Status: active
Logging: on (low)
Default: deny (incoming), allow (outgoing), disabled (routed)
New profiles: skip


To                         Action      From
--                         ------      ----
<PEER_IP>               ALLOW IN    Anywhere
```

Press Ctrl + D to exit the root shell (the next commands does not need to be run in the root shell)
```
docker exec -it fit3143 bash -c 'mpirun --version; mpicc --version; ss -tlnp | grep 2222'
docker exec -it fit3143 bash -c 'ufw status verbose'
```

## 7. Configure passwordless SSH

Inside the container, generate and display a key to exchange with the peer:

```powershell
docker exec -it fit3143 bash
mkdir -p ~/.ssh && chmod 700 ~/.ssh
ssh-keygen -t ed25519 -f ~/.ssh/id_ed25519_mpi -N ''
cat ~/.ssh/id_ed25519_mpi.pub
```

After receiving the peer public key, add it and configure SSH:

```bash
echo '<PEER_PUBLIC_KEY>' >> ~/.ssh/authorized_keys
printf 'Host mpi-a mpi-b\n    Port 2222\n    User student\n    IdentityFile ~/.ssh/id_ed25519_mpi\n    IdentitiesOnly yes\n    BatchMode yes\n    StrictHostKeyChecking accept-new\n' > ~/.ssh/config
chmod 600 ~/.ssh/authorized_keys ~/.ssh/config ~/.ssh/id_ed25519_mpi
ssh mpi-a hostname
```

## 8. Prepare and compile the program

On the Windows host, copy `task1.c` into the mounted project directory. On the launcher container (`mpi-a`), use the PDF compilation and job-submission sequence:

```powershell
Copy-Item C:\path\to\task1.c "$HOME\mpi-cluster\task1.c"
docker exec -it fit3143 bash
```

```bash
cd /home/student/mpi-cluster
scp task1.c mpi-b:/home/student/mpi-cluster/task1.c
mpicc -Wall -Wextra -O2 -o helloworld task1.c
ssh mpi-b 'cd /home/student/mpi-cluster && mpicc -Wall -Wextra -O2 -o helloworld task1.c'
```

## 9. Create the MPI hostfile and run the jobs

```bash
printf 'mpi-a slots=2\nmpi-b slots=2\n' > hosts
ip -br addr show
mpirun --hostfile hosts -np 4 --map-by ppr:2:node --mca btl_tcp_if_include <WIFI_INTERFACE> --mca oob_tcp_if_include <WIFI_INTERFACE> hostname
mpirun --hostfile hosts -np 4 --map-by ppr:2:node --mca btl_tcp_if_include <WIFI_INTERFACE> --mca oob_tcp_if_include <WIFI_INTERFACE> ./helloworld
```

## 10. Stop and restart

Run these in Administrator PowerShell on the Windows host:

```powershell
docker stop fit3143
docker start fit3143
docker exec -it fit3143 bash
docker exec fit3143 bash -c 'ss -tlnp | grep 2222'
```

## 11. Troubleshooting

- `ssh` asks for a password: check `~/.ssh/authorized_keys`, `~/.ssh/config`, port `2222`, and key permissions.
- SSH is refused: repeat the SSH-server command and check that port `2222` is listening.
- `mpirun` cannot find `helloworld`: verify `/home/student/mpi-cluster/helloworld` exists on both containers.
- The job hangs: re-check host ping, peer aliases, SSH in both directions, the host firewall, and the detected `<WIFI_INTERFACE>`.
