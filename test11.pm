program  test11;
type 
	s = array[1..10] of integer;
	t = record f, g:integer end;
var
	a:integer; b, c:S; d, e:T;

begin
	b[1] := 6; c:= b; write(c[1]);
	d.g := 7; e:= d; write(e.g);
end.

