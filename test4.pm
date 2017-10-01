program test4;
type s = record f, g:boolean end;
var v:s;

procedure p(x:integer);
const n = 10;
type t = array[1..n] of integer;
var y, z:t;

procedure q;
begin read(x); v.g := false end;

begin
	y := z;
	q;
	p(5);
	write(x);
end;

begin
	v.f := true;
	p(5)
end.

