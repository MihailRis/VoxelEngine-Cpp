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

local function intToByte(num)
	return bit.band(num, 0xFF)
end

local function reverse(tab)
    for i = 1, math.floor(#tab, 2), 1 do
        tab[i], tab[#tab-i+1] = tab[#tab-i+1], tab[i]
    end
    return tab
end

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
          table.insert(bytes, math.floor(val) % (2 ^ 8))
          val = math.floor(val / (2 ^ 8))
        end
      else
        table.insert(bytes, math.floor(mantissa) % (2 ^ 8))
        val = math.floor(mantissa / (2 ^ 8))
        table.insert(bytes, math.floor(val) % (2 ^ 8))
        val = math.floor(val / (2 ^ 8))
      end

      table.insert(bytes, math.floor(exponent * ((opt == 'd') and 16 or 128) + val) % (2 ^ 8))
      val = math.floor((exponent * ((opt == 'd') and 16 or 128) + val) / (2 ^ 8))
      table.insert(bytes, math.floor(sign * 128 + val) % (2 ^ 8))
      val = math.floor((sign * 128 + val) / (2 ^ 8))

      if not endianness then
        reverse(bytes)
      end
      return bytes
end

local function bytesToFloatOrDouble(bytes, opt)
      local n = (opt == 'd') and 8 or 4

      if not endianness then
        reverse(bytes)
      end

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

function bit_converter.single_to_bytes(float)
	return floatOrDoubleToBytes(float, 'f')
end

function bit_converter.double_to_bytes(double)
	return floatOrDoubleToBytes(double, 'd')
end

function bit_converter.uint32_to_bytes(int)
	if int > MAX_UINT32 or int < MIN_UINT32 then
		error("invalid uint32")
	end

	return {
		intToByte(bit.rshift(int, 24)),
		intToByte(bit.rshift(int, 16)),
		intToByte(bit.rshift(int, 8)),
		intToByte(int)
	}
end

function bit_converter.uint16_to_bytes(int)
	if int > MAX_UINT16 or int < MIN_UINT16 then
		error("invalid uint16")
	end

	return {
		intToByte(bit.rshift(int, 8)),
		intToByte(int)
	}
end

function bit_converter.int64_to_bytes(int)
	if int > MAX_INT64 or int < MIN_INT64 then
		error("invalid int64")
	end

	return {
		intToByte(bit.rshift(int, 56)),
		intToByte(bit.rshift(int, 48)),
		intToByte(bit.rshift(int, 40)),
		intToByte(bit.rshift(int, 32)),
		intToByte(bit.rshift(int, 24)),
		intToByte(bit.rshift(int, 16)),
		intToByte(bit.rshift(int, 8)),
		intToByte(int)
	}
end

function bit_converter.int32_to_bytes(int)
	if int > MAX_INT32 or int < MIN_INT32 then
		error("invalid int32")
	end

	return bit_converter.uint32_to_bytes(int + MAX_INT32)
end

function bit_converter.int16_to_bytes(int)
	if int > MAX_INT16 or int < MIN_INT16 then
		error("invalid int16")
	end

	return bit_converter.uint16_to_bytes(int + MAX_INT16)
end

function bit_converter.bytes_to_single(bytes)
	return bytesToFloatOrDouble(bytes, 'f')
end

function bit_converter.bytes_to_double(bytes)
	return bytesToFloatOrDouble(bytes, 'd')
end

function bit_converter.bytes_to_string(bytes)
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

function bit_converter.bytes_to_float(bytes)
	if #bytes < 8 then
		error("eof")
	end
	error("unsupported operation")
end

function bit_converter.bytes_to_uint32(bytes)
	if #bytes < 4 then
		error("eof")
	end
     return
     bit.bor(
     bit.bor(
     bit.bor(
     bit.lshift(bytes[1], 24),
     bit.lshift(bytes[2], 16)),
     bit.lshift(bytes[3], 8)),bytes[4])
end

function bit_converter.bytes_to_uint16(bytes)
	if #bytes < 2 then
		error("eof")
	end
     return
     bit.bor(
     bit.lshift(bytes[1], 8),
     bytes[2], 0)
end

function bit_converter.bytes_to_int64(bytes)
	if #bytes < 8 then
		error("eof")
	end
     return
     bit.bor(
     bit.bor(
     bit.bor(
     bit.bor(
     bit.bor(
     bit.bor(
     bit.bor(
     bit.lshift(bytes[1], 56),
     bit.lshift(bytes[2], 48)),
     bit.lshift(bytes[3], 40)),
     bit.lshift(bytes[4], 32)),
     bit.lshift(bytes[5], 24)),
     bit.lshift(bit.band(bytes[6], 0xFF), 16)),
     bit.lshift(bit.band(bytes[7], 0xFF), 8)),bit.band(bytes[8], 0xFF))
end

function bit_converter.bytes_to_int32(bytes)
	return bit_converter.bytes_to_uint32(bytes) - MAX_INT32
end

function bit_converter.bytes_to_int16(bytes)
	return bit_converter.bytes_to_uint16(bytes) - MAX_INT16
end

return bit_converter
