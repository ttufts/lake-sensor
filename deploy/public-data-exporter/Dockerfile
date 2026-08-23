FROM python:3.12-alpine

RUN apk add --no-cache git openssh-client tzdata
COPY scripts/export_public_data.py /usr/local/bin/export_public_data.py
COPY deploy/public-data-exporter/run.sh /usr/local/bin/run-public-data-exporter
RUN chmod 0755 /usr/local/bin/export_public_data.py /usr/local/bin/run-public-data-exporter \
    && mkdir -p /root/.ssh \
    && printf '%s\n' 'github.com ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIOMqqnkVzrm0SdG6UOoqKLsabgH5C9okWi0dh2l9GKJl' > /root/.ssh/known_hosts

ENTRYPOINT ["/usr/local/bin/run-public-data-exporter"]
