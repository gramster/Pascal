{ Quicksort of ten numbers }

program test10;
const max=10;
type t = array[1..max] of integer;
var a:t; k:integer;

	procedure quicksort(m, n:integer);
	var i, j:integer;

		procedure partition;
		var r, w:integer;
		begin
			r := a[(m+n) div 2];
			i := m;
			j := n;
			while (i<=j) do
			begin
				while a[i] < r do i:=i+1;
				while r < a[j] do j:=j-1;
				if i<=j then
				begin
					w:=a[i]; a[i]:=a[j]; a[j]:=w;
					i:=i+1;
					j:=j-1
				end
			end
		end;

	begin
		if m < n then
		begin
			partition;
			quicksort(m,j);
			quicksort(i,n)
		end
	end;

begin
	k:=1;
	while k<=max do
	begin
		read(a[k]);
		k := k+1
	end;		 
	quicksort(1,max);
	k:=1;
	while k<=max do
	begin
		write(a[k]);
		k := k+1
	end
end.


