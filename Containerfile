FROM docker.io/node:alpine3.20

EXPOSE 6969
WORKDIR /home/node

RUN apk add ngspice
COPY rest/* .

RUN npm install 
# RUN npm run build
RUN npx tsc

CMD ["node", "dist/index.js"]