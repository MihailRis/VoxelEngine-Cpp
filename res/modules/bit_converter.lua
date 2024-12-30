local bit_converter = { }

local MAX_UINT16 = 65535
local MIN_UINT16 = 0
local MAX_UINT32 = 4294967295
local MIN_UINT32 = 0

local MAX_INT16 = 32767
local MIN_INT16 = -32768
local MAX_INT32 = 2147483647
local MIN_INT32 = -2147483648
local MAX_INT64 = 9223372036854775807
local MIN_INT64 = -9223372036854775808

local function maskHighBytes(num)
    return bit.band(num, 0xFF)
end

local function reverse(tbl)
      for i=1, math.floor(#tbl / 2) do
        local tmp = tbl[i]
        tbl[i] = tbl[#tbl - i + 1]
        tbl[#tbl - i + 1] = tmp
      end
    return tbl
end

local orders = { "LE", "BE" }

local fromLEConvertors =
{
        LE = function(bytes) return bytes end,
        BE = function(bytes) return reverse(bytes) end
}

local toLEConvertors =
{
        LE = function(bytes) return bytes end,
        BE = function(bytes) return reverse(bytes) end
}

bit_converter.default_order = "BE"

local function fromLE(bytes, orderTo)
    if orderTo then
        bit_converter.validate_order(orderTo)
        return fromLEConvertors[orderTo](bytes)
    else return bytes end
end

local function toLE(bytes, orderFrom)
    if orderFrom then
        bit_converter.validate_order(orderFrom)
        return toLEConvertors[orderFrom](bytes)
    else return bytes end
end

function bit_converter.validate_order(order)
    if not bit_converter.is_valid_order(order) then
         error("invalid order: "..order)
    end
end

function bit_converter.is_valid_order(order) return table.has(orders, order) end

function bit_converter.string_to_bytes(str)
    local bytes = { }

    local len = string.len(str)

    local lenBytes = bit_converter.uint16_to_bytes(len)

    for i = 1, #lenBytes do
        bytes[i] = lenBytes[i]
    end

    for i = 1, len do
        bytes[#bytes + 1] = string.byte(string.sub(str, i, i))
    end

    return bytes
end

function bit_converter.bool_to_byte(bool)
    return bool and 1 or 0
end

-- Credits to Iryont <https://github.com/iryont/lua-struct>

local function floatOrDoubleToBytes(val, opt)
      local sign = 0

      if val < 0 then
        sign = 1
        val = -val
      end

      local mantissa, exponent = math.frexp(val)
      if val == 0 then
        mantissa = 0
        exponent = 0
      else
        mantissa = (mantissa * 2 - 1) * math.ldexp(0.5, (opt == 'd') and 53 or 24)
        exponent = exponent + ((opt == 'd') and 1022 or 126)
      end

      local bytes = {}
      if opt == 'd' then
        val = mantissa
        for i = 1, 6 do
          bytes[#bytes + 1] = math.floor(val) % (2 ^ 8)
          val = math.floor(val / (2 ^ 8))
        end
      else
        bytes[#bytes + 1] = math.floor(mantissa) % (2 ^ 8)
        val = math.floor(mantissa / (2 ^ 8))
        bytes[#bytes + 1] = math.floor(val) % (2 ^ 8)
        val = math.floor(val / (2 ^ 8))
      end

      bytes[#bytes + 1] = math.floor(exponent * ((opt == 'd') and 16 or 128) + val) % (2 ^ 8)
      val = math.floor((exponent * ((opt == 'd') and 16 or 128) + val) / (2 ^ 8))
      bytes[#bytes + 1] = math.floor(sign * 128 + val) % (2 ^ 8)
      val = math.floor((sign * 128 + val) / (2 ^ 8))

      return bytes
end

local function bytesToFloatOrDouble(bytes, opt)
      local n = (opt == 'd') and 8 or 4

      local sign = 1
      local mantissa = bytes[n - 1] % ((opt == 'd') and 16 or 128)
      for i = n - 2, 1, -1 do
        mantissa = mantissa * (2 ^ 8) + bytes[i]
      end

      if bytes[n] > 127 then
        sign = -1
      end

      local exponent = (bytes[n] % 128) * ((opt == 'd') and 16 or 2) + math.floor(bytes[n - 1] / ((opt == 'd') and 16 or 128))
      if exponent == 0 then
        return 0.0
      else
        mantissa = (math.ldexp(mantissa, (opt == 'd') and -52 or -23) + 1) * sign
        return math.ldexp(mantissa, exponent - ((opt == 'd') and 1023 or 127))
      end
end

--

function bit_converter.float32_to_bytes(float, order)
    return fromLE(floatOrDoubleToBytes(float, 'f'), order)
end

function bit_converter.float64_to_bytes(float, order)
    return fromLE(floatOrDoubleToBytes(float, 'd'), order)
end

function bit_converter.single_to_bytes(float, order)
    on_deprecated_call("bit_converter.float_to_bytes", "bit_converter.float32_to_bytes")
    return bit_converter.float32_to_bytes(bytes, order)
end

function bit_converter.double_to_bytes(double, order)
    on_deprecated_call("bit_converter.double_to_bytes", "bit_converter.float64_to_bytes")
    return bit_converter.float64_to_bytes(bytes, order)
end

local function uint32ToBytes(int, order)
    return fromLE({
        maskHighBytes(int),
        maskHighBytes(bit.rshift(int, 8)),
        maskHighBytes(bit.rshift(int, 16)),
        maskHighBytes(bit.rshift(int, 24))
    }, order)
end

local function uint16ToBytes(int, order)
    return fromLE({
        maskHighBytes(int),
        maskHighBytes(bit.rshift(int, 8))
    }, order)
end

function bit_converter.uint32_to_bytes(int, order)
    if int > MAX_UINT32 or int < MIN_UINT32 then
        error("invalid uint32")
    end

    return uint32ToBytes(int, order)
end

function bit_converter.uint16_to_bytes(int, order)
    if int > MAX_UINT16 or int < MIN_UINT16 then
        error("invalid uint16")
    end

    return uint16ToBytes(int, order)
end

function bit_converter.int64_to_bytes(int, order)
    if int > MAX_INT64 or int < MIN_INT64 then
        error("invalid int64")
    end

    return fromLE({
        maskHighBytes(int),
        maskHighBytes(bit.rshift(int, 8)),
        maskHighBytes(bit.rshift(int, 16)),
        maskHighBytes(bit.rshift(int, 24)),
        maskHighBytes(bit.rshift(int, 32)),
        maskHighBytes(bit.rshift(int, 40)),
        maskHighBytes(bit.rshift(int, 48)),
        maskHighBytes(bit.rshift(int, 56))
    }, order)
end

function bit_converter.int32_to_bytes(int, order)
    on_deprecated_call("bit_converter.int32_to_bytes", "bit_converter.sint32_to_bytes")

    if int > MAX_INT32 or int < MIN_INT32 then
        error("invalid int32")
    end

    return uint32ToBytes(int + MAX_INT32, order)
end

function bit_converter.int16_to_bytes(int, order)
    on_deprecated_call("bit_converter.int32_to_bytes", "bit_converter.sint16_to_bytes")

    if int > MAX_INT16 or int < MIN_INT16 then
        error("invalid int16")
    end

    return uint16ToBytes(int + MAX_INT16, order)
end

function bit_converter.sint32_to_bytes(int, order)
    if int > MAX_INT32 or int < MIN_INT32 then
        error("invalid sint32")
    end

    return uint32ToBytes(int + MAX_UINT32 + 1, order)
end

function bit_converter.sint16_to_bytes(int, order)
    if int > MAX_INT16 or int < MIN_INT16 then
        error("invalid sint16")
    end

    return uint16ToBytes(int + MAX_UINT16 + 1, order)
end

function bit_converter.bytes_to_float32(bytes, order)
    return bytesToFloatOrDouble(toLE(bytes, order), 'f')
end

function bit_converter.bytes_to_float64(bytes, order)
    return bytesToFloatOrDouble(toLE(bytes, order), 'd')
end

function bit_converter.bytes_to_single(bytes, order)
    on_deprecated_call("bit_converter.bytes_to_single", "bit_converter.bytes_to_float32")
    return bit_converter.bytes_to_float32(bytes, order)
end

function bit_converter.bytes_to_double(bytes, order)
    on_deprecated_call("bit_converter.bytes_to_double", "bit_converter.bytes_to_float64")
    return bit_converter.bytes_to_float64(bytes, order)
end

function bit_converter.bytes_to_string(bytes, order)
    local len = bit_converter.bytes_to_uint16({ bytes[1], bytes[2] })

    local str = ""

    for i = 1, len do
        str = str..string.char(bytes[i + 2])
    end

    return str
end

function bit_converter.byte_to_bool(byte)
    return byte ~= 0
end

function bit_converter.bytes_to_uint32(bytes, order)
    if #bytes < 4 then
        error("eof")
    end

    bytes = toLE(bytes, order)

     return
     bit.bor(
     bit.bor(
     bit.bor(
     bytes[1],
     bit.lshift(bytes[2], 8)),
     bit.lshift(bytes[3], 16)),
     bit.lshift(bytes[4], 24))
end

function bit_converter.bytes_to_uint16(bytes, order)
    if #bytes < 2 then
        error("eof")
    end

    bytes = toLE(bytes, order)

     return
     bit.bor(
     bit.lshift(bytes[2], 8),
     bytes[1], 0)
end

function bit_converter.bytes_to_int64(bytes, order)
    if #bytes < 8 then
        error("eof")
    end

    bytes = toLE(bytes, order)

     return
     bit.bor(
     bit.bor(
     bit.bor(
     bit.bor(
     bit.bor(
     bit.bor(
     bit.bor(
     bit.lshift(bytes[8], 56),
     bit.lshift(bytes[7], 48)),
     bit.lshift(bytes[6], 40)),
     bit.lshift(bytes[5], 32)),
     bit.lshift(bytes[4], 24)),
     bit.lshift(bit.band(bytes[3], 0xFF), 16)),
     bit.lshift(bit.band(bytes[2], 0xFF), 8)),bit.band(bytes[1], 0xFF))
end

function bit_converter.bytes_to_int32(bytes, order)
    on_deprecated_call("bit_converter.bytes_to_int32", "bit_converter.bytes_to_sint32")
    return bit_converter.bytes_to_uint32(bytes, order) - MAX_INT32
end

function bit_converter.bytes_to_int16(bytes, order)
    on_deprecated_call("bit_converter.bytes_to_int16", "bit_converter.bytes_to_sint16")
    return bit_converter.bytes_to_uint16(bytes, order) - MAX_INT16
end

function bit_converter.bytes_to_sint32(bytes, order)
    local num = bit_converter.bytes_to_uint32(bytes, order)

    return MIN_INT32 * (bit.band(MAX_INT32 + 1, num) ~= 0 and 1 or 0) + bit.band(MAX_INT32, num)
end

function bit_converter.bytes_to_sint16(bytes, order)
    local num = bit_converter.bytes_to_uint16(bytes, order)

    return MIN_INT16 * (bit.band(MAX_INT16 + 1, num) ~= 0 and 1 or 0) + bit.band(MAX_INT16, num)
end

return bit_converter
