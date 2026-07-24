local echo = require("echo")

local resp = echo.EchoService.Echo({ message = "plain", color = "BLUE" })

assert(resp.message == "plain", "message mismatch")
assert(resp.length == 5, "length mismatch")
assert(resp.color == "BLUE", "color mismatch")

print("echo_plain_table: OK", resp.message, resp.length, resp.color)
