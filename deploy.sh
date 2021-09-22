docker build -t cpp-api .
docker rm -f cpp-api
docker run -it -d --network dbnetwork -p 80:8000 --name cpp-api cpp-api
docker logs cpp-api
# docker exec -it cpp-api /bin/sh