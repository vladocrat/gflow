local seen = 0

for tick in EchoService.Subscribe(TickRequest { ticks = 5, interval_ms = 50 }) do
    print(string.format("seq=%d label=%s", tick.seq, tick.label))
    assert(tick.seq == seen, "out-of-order tick")
    seen = seen + 1
end

assert(seen == 5, "expected 5 ticks, got " .. seen)
print("subscribe: OK")
