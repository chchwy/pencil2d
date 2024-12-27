FROM ubuntu:16.04

# Install dependencies
RUN apt-get update -yq && \
    apt-get install -y \
    software-properties-common && \
    apt-add-repository -y ppa:git-core/ppa && \
    apt-get update -yq && \
    apt-get install -yq --no-install-suggests --no-install-recommends git tree curl 

# Copy any scripts you need
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]