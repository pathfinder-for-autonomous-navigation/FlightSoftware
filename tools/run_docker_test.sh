# Create docker container for running flight software tests
docker build -t fswbase -f tools/Dockerfile.base .
docker build -t fswtest -f tools/Dockerfile.test .
