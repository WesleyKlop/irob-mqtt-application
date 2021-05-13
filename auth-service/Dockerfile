FROM node:16

WORKDIR /app

COPY ./ /app
RUN npm install

ENTRYPOINT ["/usr/local/bin/node", "--experimental-json-modules"]
CMD ["/app/server.mjs"]