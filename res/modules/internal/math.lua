function math.Rand( low, high )
    return low + (high - low) * math.random()
end

function math.Clamp( _in, low, high )
    return math.min(math.max( _in, low ), high)
end

function math.Lerp(frac, from, to)
    return from + (to - from) * frac
end