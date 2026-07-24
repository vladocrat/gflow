local echo = require("echo")

local resp = echo.EchoService.Echo(echo.EchoRequest {
    message = "hello from lua",
    count = 3,
    loud = true,
    color = "GREEN",
})

print("message:", resp.message)
print("length:", resp.length)
print("color:", resp.color)

assert(resp.message == "hello from lua", "message mismatch")
assert(resp.length == #"hello from lua", "length mismatch")
assert(resp.color == "GREEN", "color mismatch")

print("echo_hello: OK")
