local echo = require("echo")

local req = echo.EchoRequest {
    message = "hello",
    count = 3,
    loud = true,
    ratio = 1.5,
    color = "GREEN",
    origin = { x = 1, y = 2 },
    tags = { "alpha", "beta" },
    path = { { x = 0, y = 0 }, { x = 5, y = 5 } },
    counters = { hits = 10, misses = 2 },
    text_payload = "set",
}

local resp = echo.EchoService.Echo(req)
print(resp.message, resp.length, resp.color)

for tick in echo.EchoService.Subscribe(echo.TickRequest { ticks = 5, interval_ms = 100 }) do
    print(tick.seq, tick.label)
end
