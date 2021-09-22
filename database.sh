docker pull mongo

# removing old
docker network disconnect dbnetwork mongodb
docker rm -f mongodb
docker network rm dbnetwork

# starting new
docker run --name mongodb -d mongo
docker network create dbnetwork
docker network connect dbnetwork mongodb