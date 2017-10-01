program test;
const
	four = 4; { try a { nested comment } }
type
	tbl = array[1..four] of integer;
var
	a: integer;
	z: tbl;

procedure factorial(var v:integer);
var
	t: integer;
begin
	if (v = 0) then v := 1
	else begin
		t := v - 1;
		factorial(t);
		v := v * t
	end
end;

begin
	read(a);
	factorial(a);
	write(a)
end.

