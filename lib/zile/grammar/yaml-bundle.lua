{
  name = 'YAML',
  fileTypes = { 'yaml', 'yml', 'conf' },
  scopeName = 'source.yaml',
  foldingStartMarker = [[^[^#]\s*.*:(\s*\[?| &.+)?$]],
  foldingStopMarker = [[^\s*$|^\s*\}|^\s*\]|^\s*\)]],
  keyEquivalent = '^~Y',
  repository = {
    escaped_char = {
      name = 'constant.character.escape.yaml',
      match = [[\\.]],
    },
    unquoted_block = {
      name = 'string.unquoted.block.yaml',
      begin = [[^(\s*)(?:(-)|(?:(-\s*)?(\w+\s*(:))))(?:\s*(\||>))?]],
      beginCaptures = {
        [2] = { name = 'punctuation.definition.entry.yaml' },
        [3] = { name = 'punctuation.definition.entry.yaml' },
        [4] = { name = 'entity.name.tag.yaml' },
        [5] = { name = 'punctuation.separator.key-value.yaml' },
      },
      ['end'] = [[^(?!^\1)|^(?=\1(-|\w+\s*:)|#)]],
      patterns = {
        { include = '#unquoted_block' },
        { include = 'source.lua' },
      },
    },
  },
  patterns = {
    { include = '#unquoted_block' },
    {
      name = 'constant.numeric.yaml',
      match = [[(?:(?:(-\s*)?(\w+\s*(:)))|(-))\s*((0(x|X)[0-9a-fA-F]*)|(([0-9]+\.?[0-9]*)|(\.[0-9]+))((e|E)(\+|-)?[0-9]+)?)(L|l|UL|ul|u|U|F|f)?\s*$]],
      captures = {
        [1] = { name = 'punctuation.definition.entry.yaml' },
        [2] = { name = 'entity.name.tag.yaml' },
        [3] = { name = 'punctuation.separator.key-value.yaml' },
        [4] = { name = 'punctuation.definition.entry.yaml' },
      },
    },
    {
      name = 'string.unquoted.yaml',
      match = [[(?:(?:(-\s*)?(\w+\s*(:)))|(-))\s*([A-Za-z0-9].*)\s*$]],
      captures = {
        [1] = { name = 'punctuation.definition.entry.yaml' },
        [2] = { name = 'entity.name.tag.yaml' },
        [3] = { name = 'punctuation.separator.key-value.yaml' },
        [4] = { name = 'punctuation.definition.entry.yaml' },
        [5] = { name = 'string.unquoted.yaml' },
      },
    },
    {
      name = 'constant.other.date.yaml',
      match = [[(?:(?:(-\s*)?(\w+\s*(:)))|(-))\s*([0-9]{4}-[0-9]{2}-[0-9]{2})\s*$]],
      captures = {
        [1] = { name = 'punctuation.definition.entry.yaml' },
        [2] = { name = 'entity.name.tag.yaml' },
        [3] = { name = 'punctuation.separator.key-value.yaml' },
        [4] = { name = 'punctuation.definition.entry.yaml' },
      },
    },
    {
      name = 'meta.tag.yaml',
      match = [[(\w.*?)(:)\s*((\!\!)omap)?]],
      captures = {
        [1] = { name = 'entity.name.tag.yaml' },
        [2] = { name = 'punctuation.separator.key-value.yaml' },
        [3] = { name = 'keyword.other.omap.yaml' },
        [4] = { name = 'punctuation.definition.keyword.yaml' },
      },
    },
    {
      name = 'variable.other.yaml',
      match = [[(\&|\*)\w.*?$]],
      captures = {
        [1] = { name = 'punctuation.definition.variable.yaml' },
      },
    },
    {
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
    {
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
    {
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
    {
      name = 'keyword.operator.merge-key.yaml',
      match = [[(\\<\\<) = ((\\*).*)$]],
      captures = {
        [1] = { name = 'entity.name.tag.yaml' },
        [2] = { name = 'keyword.operator.merge-key.yaml' },
        [3] = { name = 'punctuation.definition.keyword.yaml' },
      },
    },
    {
      name = 'invalid.deprecated.trailing-whitespace.yaml',
      match = [[(\s+)$]],
    },
    {
      name = 'comment.line.number-sign.yaml',
      match = [[(?<!\$)(#)(?!\{).*$\n?]],
      captures = {
        [1] = { name = 'punctuation.definition.comment.yaml' },
      },
    },
    {
      name = 'keyword.operator.symbol',
      match = [[-]],
    },
    {
      name = 'invalid.leading-tabs.yaml',
      match = [[^(\t+)]],
    },
  },
}
