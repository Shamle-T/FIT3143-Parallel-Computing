FROM ubuntu:24.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends openmpi-bin libopenmpi-dev build-essential openssh-server python3 ca-certificates && rm -rf /var/lib/apt/lists/*
RUN mkdir -p /run/sshd /root/.ssh && ssh-keygen -t ed25519 -N '' -f /root/.ssh/id_ed25519 && cp /root/.ssh/id_ed25519.pub /root/.ssh/authorized_keys && chmod 600 /root/.ssh/authorized_keys && printf 'Host *\n  StrictHostKeyChecking no\n  UserKnownHostsFile /dev/null\n  LogLevel ERROR\n' > /root/.ssh/config
WORKDIR /lab
CMD ["/usr/sbin/sshd", "-D"]
