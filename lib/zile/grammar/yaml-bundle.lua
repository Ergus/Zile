{
  name = 'YAML',
  fileTypes = { 'yaml', 'yml', 'conf' },
  scopeName = 'source.yaml',
  foldingStartMarker = [[^[^#]\s*.*:(\s*\[?| &.+)?$]],
  foldingStopMarker = [[^\s*$|^\s*\}|^\s*\]|^\s*\)]],
  keyEquivalent = '^~Y',
  repository = {
    comment = {
      name = 'comment.line.number-sign.yaml',
      match = [[(?<!\$)(#)(?!\{).*$\n?]],
      captures = {
        [1] = { name = 'punctuation.definition.comment.yaml' },
      },
    },
    date = {
      name = 'constant.other.date.yaml',
      match = [[(?:(?:(-\s*)?(\w+\s*(:)))|(-))\s*([0-9]{4}-[0-9]{2}-[0-9]{2})\s*$]],
      captures = {
        [1] = { name = 'punctuation.definition.entry.yaml' },
        [2] = { name = 'entity.name.tag.yaml' },
        [3] = { name = 'punctuation.separator.key-value.yaml' },
        [4] = { name = 'punctuation.definition.entry.yaml' },
      },
    },
    double_quote = {
      name = 'string.quoted.double.yaml',
      begin = [["]],
      beginCaptures = {
        [0] = { name = 'punctuation.definition.string.begin.yaml' },
      },
      ['end'] = [["]],
      endCaptures = {
        [0] = { name = 'punctuation.definition.string.end.yaml' },
      },
      patterns = {
        { include = '#escaped_char' },
      },
    },
    escaped_char = {
      name = 'constant.character.escape.yaml',
      match = [[\\.]],
    },
    hyphen = {
      name = 'keyword.operator.symbol',
      match = [[-]],
    },
    interpolated = {
      name = 'string.interpolated.yaml',
      begin = [[`]],
      beginCaptures = {
        [0] = { name = 'punctuation.definition.string.begin.yaml' },
      },
      ['end'] = [[`]],
      endCaptures = {
        [0] = { name = 'punctuation.definition.string.end.yaml' },
      },
      patterns = {
        { include = '#escaped_char' },
      },
    },
    invalid_leading_tabs = {
      name = 'invalid.leading-tabs.yaml',
      match = [[^(\t+)]],
    },
    invalid_trailing_whitespace = {
      name = 'invalid.deprecated.trailing-whitespace.yaml',
      match = [[([ \t]+)$]],
    },
    lua = {
      name = 'source.embedded.lua',
      begin = [[^(\s+)(?![-#\s])]],
      ['end'] = [[^(?!^\1)|^(?=\1(-|\w+\s*:))]],
      patterns = {
        {  include = 'source.lua' },
      },
    },
    merge_key = {
      name = 'keyword.operator.merge-key.yaml',
      match = [[(\\<\\<) = ((\\*).*)$]],
      captures = {
        [1] = { name = 'entity.name.tag.yaml' },
        [2] = { name = 'keyword.operator.merge-key.yaml' },
        [3] = { name = 'punctuation.definition.keyword.yaml' },
      },
    },
    numeric_constant = {
      name = 'constant.numeric.yaml',
      match = [[(?:(?:(-\s*)?(\w+\s*(:)))|(-))\s*((0(x|X)[0-9a-fA-F]*)|(([0-9]+\.?[0-9]*)|(\.[0-9]+))((e|E)(\+|-)?[0-9]+)?)(L|l|UL|ul|u|U|F|f)?\s*$]],
      captures = {
        [1] = { name = 'punctuation.definition.entry.yaml' },
        [2] = { name = 'entity.name.tag.yaml' },
        [3] = { name = 'punctuation.separator.key-value.yaml' },
        [4] = { name = 'punctuation.definition.entry.yaml' },
      },
    },
    single_quote = {
      name = 'string.quoted.single.yaml',
      begin = [[']],
      beginCaptures = {
        [0] = { name = 'punctuation.definition.string.begin.yaml' },
      },
      ['end'] = [[']],
      endCaptures = {
        [0] = { name = 'punctuation.definition.string.end.yaml' },
      },
      patterns = {
        { include = '#escaped_char' },
      },
    },
    tag = {
      name = 'meta.tag.yaml',
      match = [[(\w.*?)(:)\s*((\!\!)omap)?]],
      captures = {
        [1] = { name = 'entity.name.tag.yaml' },
        [2] = { name = 'punctuation.separator.key-value.yaml' },
        [3] = { name = 'keyword.other.omap.yaml' },
        [4] = { name = 'punctuation.definition.keyword.yaml' },
      },
    },
    unquoted_block = {
      name = 'string.unquoted.block.yaml',
      begin = [[^(\s*)(?:(-)|(?:(-\s*)?(\S[^:]*\s*(:))))(?:\s*(\||>|$))]],
      beginCaptures = {
        [2] = { name = 'punctuation.definition.entry.yaml' },
        [3] = { name = 'punctuation.definition.entry.yaml' },
        [4] = { name = 'entity.name.tag.yaml' },
        [5] = { name = 'punctuation.separator.key-value.yaml' },
        [6] = { name = 'punctuation.style.flow.yaml' },
      },
      ['end'] = [[^(?!^\1)|^(?=\1(-|\w+\s*:)|#)]],
      patterns = {
        { include = '#comment' },
        { include = '#unquoted_block' },
        { include = '#numeric_constant' },
        { include = '#unquoted_string' },
        { include = '#date' },
        { include = '#tag' },
        { include = '#variable' },
        { include = '#double_quote' },
        { include = '#single_quote' },
        { include = '#interpolated' },
        { include = '#merge_key' },
        { include = '#invalid_trailing_whitespace' },
        { include = '#hyphen' },
        { include = '#invalid_leading_tabs' },
        { include = '#lua' },
      },
    },
    unquoted_string = {
      name = 'string.unquoted.yaml',
      match = [=[(?:(?:(-\s*)?(\w+\s*(:)))|(-))\s*(?:((")[^"]*("))|((')[^']*('))|([^,{}&#\[\]]+))\s*]=],
      captures = {
        [1] = { name = 'punctuation.definition.entry.yaml' },
        [2] = { name = 'entity.name.tag.yaml' },
        [3] = { name = 'punctuation.separator.key-value.yaml' },
        [4] = { name = 'punctuation.definition.entry.yaml' },
        [5] = { name = 'string.quoted.double.yaml' },
        [6] = { name = 'punctuation.definition.string.begin.yaml' },
        [7] = { name = 'punctuation.definition.string.end.yaml' },
        [8] = { name = 'string.quoted.single.yaml' },
        [9] = { name = 'punctuation.definition.string.begin.yaml' },
        [10] = { name = 'punctuation.definition.string.end.yaml' },
        [11] = { name = 'string.unquoted.yaml' },
      },
    },
    variable = {
      name = 'variable.other.yaml',
      match = [[(\&|\*)\w.*?$]],
      captures = {
        [1] = { name = 'punctuation.definition.variable.yaml' },
      },
    },
  },
  patterns = {
    { include = '#comment' },
    { include = '#unquoted_block' },
    { include = '#numeric_constant' },
    { include = '#unquoted_string' },
    { include = '#date' },
    { include = '#tag' },
    { include = '#variable' },
    { include = '#double_quote' },
    { include = '#single_quote' },
    { include = '#interpolated' },
    { include = '#merge_key' },
    { include = '#invalid_trailing_whitespace' },
    { include = '#hyphen' },
    { include = '#invalid_leading_tabs' },
    { include = '#lua' },
  },
}
