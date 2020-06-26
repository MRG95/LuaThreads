--Testing

function TestWork(IN)
    local OUT = TagCompound.new()

    for i=0, 50 do

        OUT:addChild(TagInt.new("1"))
        OUT:addChild(TagInt.new("2"))
        OUT:addChild(TagInt.new("3"))
        OUT:addChild(TagInt.new("4"))
        OUT:addChild(TagInt.new("5"))
        OUT:addChild(TagInt.new("6"))
        OUT:addChild(TagInt.new("7"))
        OUT:addChild(TagInt.new("8"))
        OUT:addChild(TagInt.new("9"))
        OUT:addChild(TagInt.new("10"))
        OUT:addChild(TagInt.new("12"))
        OUT:addChild(TagInt.new("13"))
        OUT:addChild(TagInt.new("14"))
        OUT:addChild(TagInt.new("15"))
        OUT:addChild(TagInt.new("16"))
        OUT:addChild(TagInt.new("17"))
        OUT:addChild(TagInt.new("18"))
        OUT:addChild(TagInt.new("19"))
        OUT:addChild(TagInt.new("20"))

    end

    return OUT
end