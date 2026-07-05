local EchoService = require("echo")

local req = EchoRequest {
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

local resp = EchoService.Echo(req)
print(resp.message, resp.length, resp.color)

for tick in EchoService.Subscribe(TickRequest { ticks = 5, interval_ms = 100 }) do
    print(tick.seq, tick.label)
end
