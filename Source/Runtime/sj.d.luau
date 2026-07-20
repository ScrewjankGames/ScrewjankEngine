
declare extern type Mat44 with
    x: vector
    y: vector
    z: vector
    w: vector
end

declare extern type GameObject with
    function GetTransform(self): Mat44
end
