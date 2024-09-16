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

import Fastify from 'fastify';

const TEMP_FILE_LOCATION = process.env.TEMP_FILE_LOCATION || "/tmp"
const SPICE_COMMAND_NAME = "ngspice";
const SERVICE_PORT = Number(process.env.SERVICE_PORT) || 6969

const fastify = Fastify({logger: {level: "trace"}});

function makeTempFilename() {
  return `${TEMP_FILE_LOCATION}/spice-fifo-${ randomUUID() }`
}


async function readTempFile(
  spiceProcess: ChildProcess, 
  tempFilename: string
): Promise<number[][]> {
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
      let cleanedLines: number[][] = [];
      for await (const line of lines) {
        cleanedLines.push(line.trim().split(/\s+/g).map(parseFloat))
      }
      resolve(cleanedLines);
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

fastify.post(
  '/allData',
  async function handler (request, reply) {
    fastify.log.debug(`Request body: \n ${request.body}`)
    assert(request.body instanceof Array)
    try {
      return JSON.stringify(await Promise.all(request.body.map(runSpice)));
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
  