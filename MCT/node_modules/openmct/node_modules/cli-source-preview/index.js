'use strict'

const chalk = require('chalk')

const PREVIEW_OPTS = {
  offset: 5,
  lineNumber: true,
  delimiter: '\n',
  cliColor: true
}
const DELIMITER = '-'.repeat(40)

function rightPad (text, width) {
  return text + (width > text.length ? ' '.repeat(width - text.length) : '')
}

function colorer (cliColor) {
  return function (type, string) {
    if (cliColor) {
      return chalk[type](string)
    } else {
      return string
    }
  }
}

/**
 * Preview parts of source code
 *
 * line:
 *
 * - Array: `[ Number, Number ]`, the line range to preview
 * - Object: `{ line: Number, column: Number }`, preivew specified line/column
 * - Number: preivew single line
 *
 * options:
 *
 * - offset: the extra lines number before/after specified line range (default: 5)
 * - lineNumber: show line number or not (default: true)
 * - delimiter: line delimiter (default: '\n')
 * - cliColor: show ASCI CLI color (default: true)
 *
 * @param  {String} source  Source code
 * @param  {Mixed}  line    Line number to preivew
 * @param  {Object} options Options (optional)
 * @return {String} source parts
 */
function preview (source, line, options) {
  let from, to, pos

  // set from/to line
  if (Array.isArray(line)) {
    from = line[0] | 0
    to = line[1] | 0 || from
  } else if (typeof line === 'object') {
    from = to = line.line | 0
    pos = {
      line: line.line | 0,
      column: line.column | 0
    }
  } else {
    from = to = line | 0
  }

  // set options
  options = Object.assign({}, PREVIEW_OPTS, options)

  let color = colorer(options.cliColor)

  // read source by from/to
  let lines = readSource(source, from, to, options.offset, options.delimiter)
  let numberWidth = String(to).length + 4 // [] + two space

  if (!options.lineNumber) {
    numberWidth = 0
  }

  let parts = lines.map(line => {
    let prefix = ''
    let text = ''

    if (options.lineNumber) {
      prefix = rightPad(`[${line.number}]`, numberWidth)
    }

    if (line.number >= from && line.number <= to) {
      text = color('red', `${prefix}${line.source}`)
    } else {
      text = color('grey', prefix) + line.source
    }

    if (pos && pos.line === line.number) {
      text += '\n' + ' '.repeat(numberWidth + pos.column - 1) + '^'
    }

    return text
  })

  // add delimiter
  parts.unshift(color('grey', DELIMITER))
  parts.push(color('grey', DELIMITER))

  return parts.join('\n')
}

/**
 * Read source by line number
 *
 * Return: [
 *   {
 *   	 number: Number,
 *   	 source: String
 *   },
 *   ...
 * ]
 *
 * @param  {String} source
 * @param  {Number} from
 * @param  {Number} to (optional)
 * @param  {Number} offset (optional)
 * @param  {String} delimiter (optional)
 * @return {Array} Source splitd by line
 */
function readSource (source, from, to, offset, delimiter) {
  // fix args
  from = from | 0
  to = to | 0 || from
  delimiter = delimiter || PREVIEW_OPTS.delimiter

  if (typeof offset === 'undefined') {
    offset = PREVIEW_OPTS.offset
  } else {
    offset = offset | 0
  }

  let lastIdx = -1
  let currIdx = lastIdx
  let line = 1
  let reads = []

  from -= offset
  to += offset

  while (currIdx < source.length) {
    currIdx = source.indexOf(delimiter, lastIdx + 1)
    if (currIdx < 0) {
      currIdx = source.length
    }

    if (line > to) {
      break
    } else if (line >= from && line <= to) {
      reads.push({
        number: line,
        source: source.substring(lastIdx + delimiter.length, currIdx)
      })
    }

    lastIdx = currIdx
    line ++
  }

  return reads
}

module.exports = preview
module.exports.readSource = readSource
