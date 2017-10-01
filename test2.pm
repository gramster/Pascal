program test2;
const
	a = 1; b = a;
type
	t = array[1..2] of integer;
	u = record f, g: integer; h:boolean end;
	v = record f:integer end;
var
	x, y:t; z:u;

procedure P(var x:integer; y:boolean);
const a=1;

	procedure q(x:integer);
	type t = array[1..2] of integer;
	begin
		x := -1;
		x := x;
		x := (2-1)*(2+1) div 2 mod 2;
		if x < x then
			while x = x do q(x);
		if x>x then
			while x<=x do p(x, false)
		else
			if not (x<>x) then { empty }
	end;

begin
	if x>=x then y:= true
end;

procedure r;
var x:t;
begin x[1]:=5 end;

begin z.f := 6 end.


