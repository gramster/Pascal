program  test9;
const two = 2;
type 
	s = array[1..10] of integer;
	t = record f, g:integer end;
var
	a:integer; b, c:S; d, e:T;

procedure writebool(x:boolean);
begin
	if x then write(1) else write(0) 
end;

procedure echoone;
begin
	read(a);
	write(a)
end;

procedure p(u:integer; var v:integer);
var x:integer;
begin
	echoone;
	write(u);
	v:=3; write(a);
	x:=4; write(x)
end;

procedure q;
begin
	write(5)
end;

begin
	write(0);
	p(two, a);
	q;
	b[10] := 6; c:= b; write(c[10]);
	d.g := 7; e:= d; write(e.g);
	write(-8); write(8+1);
	write(11-1); write (22 div 2);
	write (6*2); write (27 mod 14);
	writebool(not false);
	writebool(false and true);
	writebool(false or true);
	writebool(1<2); writebool(1=2);
	writebool(1>2); writebool(1<=2);
	writebool(1<>2); writebool(1>=2);
	if true then write(14);
	if false then write(0) else write(15);
	a:=16;
	while a<=17 do
	begin
		write(a);
		a:=a+1;
	end
end.

