import assert from 'node:assert/strict';
import {
  ChildProcess,
  spawn,
} from 'node:child_process';
import { randomUUID } from 'node:crypto';
import { createReadStream } from 'node:fs';
import { rm } from 'node:fs/promises';
import * as readline from 'node:readline/promises';
import { Readable } from 'node:stream';
import { inspect } from 'node:util';

import Fastify from 'fastify';

const TEMP_FILE_LOCATION = process.env.TEMP_FILE_LOCATION || "/tmp"
const SPICE_COMMAND_NAME = "ngspice";
const SERVICE_PORT = Number(process.env.SERVICE_PORT) || 6969

const fastify = Fastify({logger: {level: "trace"}});

function makeTempFilename() {
  return `${TEMP_FILE_LOCATION}/spice-fifo-${ randomUUID() }`
}


async function readTempFile(spiceProcess: ChildProcess, tempFilename: string) {
  return new Promise(async (resolve, reject) => {
    spiceProcess.once('error', async (err) => {
      reject(err);
    })
    spiceProcess.once('exit', async () => {
      if (spiceProcess.stdout === null) {
        throw new Error("error: child process stdout is unavailable")
      }
      fastify.log.trace(`ngspice has exited. attempting to read data file.`)
      try {
      const analysisTempFile = createReadStream(tempFilename, {encoding: "utf-8"});
      const lines = readline.createInterface({input: analysisTempFile})
      let cleanedLines: string[] = [];
      for await (const line of lines) {
        cleanedLines.push(line.trim().replaceAll(/\s+/g, ","))
      }
      const cleanedData = cleanedLines.join("\n");
      fastify.log.debug(`Data file contents: ${cleanedData}`);
      resolve(cleanedData);
      } catch (err) {
        fastify.log.error(`error: error occured when reading temp file: ${err}`)
        reject(err);
      }
    })
  })
}

const runSpice = async (spiceCode: string) => {
  const tempFilename = makeTempFilename();
  fastify.log.debug(`temp file filename: ${tempFilename}`)
  const spiceProcess = spawn(
    SPICE_COMMAND_NAME, 
    ["--define", `tempFile=${tempFilename}`],
    {stdio: ["pipe", "pipe", "inherit"]}
  );
  fastify.log.trace(`starting ngspice`)
  Readable.from(spiceCode).pipe(spiceProcess.stdin);
  //process.stdin.end()
  
  const data = await readTempFile(spiceProcess, tempFilename);
  await rm(tempFilename, {force: true});
  
  return data;
}

fastify.addContentTypeParser(
  "application/spice", 
  {parseAs: "string"},
  (request, body, done) => {
    assert(typeof body === "string");
    fastify.log.debug(`Preparsed body: \n ${body}`);
    const parsedBody = body
      .split("\n")
      .map((line) => Buffer.from(line, "base64").toString("utf8"))
    fastify.log.debug(`Parsed body: \n${inspect(parsedBody)}`)
    done(null, parsedBody);
  })


fastify.post(
  '/allData',
  async function handler (request, reply) {
    fastify.log.debug(`Request body: \n ${request.body}`)
    assert(request.body instanceof Array)
    try {
      return (await Promise.all(request.body.map(runSpice))).join("\n");
    } catch(err) {
      reply.status(500);
      return err;
    }
  })
  
async function main() {
  await fastify.listen({
    port: SERVICE_PORT,
    host: "0.0.0.0"
  });
}

main().then((resolve) => {}, (reject) => fastify.log.error(reject))
  