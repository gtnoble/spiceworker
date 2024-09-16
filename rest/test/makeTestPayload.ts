import fs from 'node:fs';

const data: string[] = [];
for (const filename of process.argv.slice(2)) {
  data.push(fs.readFileSync(filename, {encoding: "utf8"}))
}

console.log(JSON.stringify(data));