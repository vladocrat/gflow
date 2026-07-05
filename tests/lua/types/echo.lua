---@meta

---@alias Color "COLOR_UNSPECIFIED"|"RED"|"GREEN"|"BLUE"

---@class (exact) Point
---@field x? integer
---@field y? integer

---@class (exact) EchoRequest
---@field message? string
---@field loud? boolean
---@field count? integer
---@field big? integer
---@field ratio? number
---@field blob? string
---@field color? Color
---@field origin? Point
---@field tags? string[]
---@field path? Point[]
---@field counters? table<string, integer>
---@field text_payload? string
---@field number_payload? integer

---@class (exact) EchoResponse
---@field message? string
---@field length? integer
---@field color? Color

---@class (exact) TickRequest
---@field ticks? integer
---@field interval_ms? integer

---@class (exact) Tick
---@field seq? integer
---@field label? string

---@param fields Point
---@return Point
function Point(fields) end

---@param fields EchoRequest
---@return EchoRequest
function EchoRequest(fields) end

---@param fields EchoResponse
---@return EchoResponse
function EchoResponse(fields) end

---@param fields TickRequest
---@return TickRequest
function TickRequest(fields) end

---@param fields Tick
---@return Tick
function Tick(fields) end

local EchoService = {}

---@param req EchoRequest
---@return EchoResponse
function EchoService.Echo(req) end

---@param req TickRequest
---@return fun():Tick
function EchoService.Subscribe(req) end

return EchoService