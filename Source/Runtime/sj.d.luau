
declare extern type mat44 with
    x: vector
    y: vector
    z: vector
    w: vector
end

declare extern type GameObject with
    function GetTransform(self): mat44
end
