# cli-source-preview

Preview source code for CLI, useful when display error message with related source.

## install

```bash
$ npm install cli-source-preview
```

## usage

```js
'use strict'

const preview = require('cli-source-preview')
const source = `
'use strict'

const chalk = require('chalk')

const PREVIEW_OPTS = {
  offset: 5,
  lineNumber: true,
  delimiter: '\\n'
}
const DELIMITER = '-'.repeat(40)

function rightPad (text, width) {
  return text + (width > text.length ? ' '.repeat(width - text.length) : '')
}
`

// preview line 10
console.log(preview(source, 10))

// preview line 5~8
console.log(preview(source, [5, 8]))

// preview line 12:6
console.log(preview(source, { line: 12, column: 6}))
```

## Screen Shot

![Screen Shot](./screen-shot.png)

## API

### `preview (source, line, options)`

Get source code parts by line and return with highlighted (CLI) string.

#### Arguments

- **source** (String), source code to preview
- **line** (Mixed), line number to highlight and preview, types:
  - Array: `[ Number, Number ]`, the line range to preview
  - Object: `{ line: Number, column: Number }`, preivew specified line/column
  - Number: preivew single line
- **options** (Object), preview options *(optional)*
  - **offset**: extra lines number before/after specified line range *(default: 5)*
  - **lineNumber**: show line number or not *(default: true)*
  - **delimiter**: line delimiter *(default: '\n')*
  - **cliColor**: show ASCI CLI highlight color, set false to turn off CLI highlight *(default: true)*

#### Return

Return preview parts string of source

### `preview.readSource (source, from, to, offset, delimiter)`

Read source code by from/to line range.

#### Arguments

- **source** (String), source code to read
- **from** (Number), from line
- **to** (Number), to line
- **offset** (Number), same as `options.offset`
- **delimiter** (String), same as `options.delimiter`

#### Return

Return readed source code lines as format:

```js
return [{
  number: Number,
  source: String
}, ...]
```

## License

MIT
