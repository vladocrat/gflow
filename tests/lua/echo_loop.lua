local echo = require("echo")

local colors = { "RED", "GREEN", "BLUE" }

for i, color in ipairs(colors) do
    local text = string.rep("x", i)
    local resp = echo.EchoService.Echo(echo.EchoRequest { message = text, color = color })
    assert(resp.length == i, "length mismatch for " .. color)
    assert(resp.color == color, "color mismatch")
    print(string.format("[%d] color=%s length=%d", i, resp.color, resp.length))
end

print("echo_loop: OK")
