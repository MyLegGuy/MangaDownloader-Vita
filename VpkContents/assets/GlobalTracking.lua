-- These two functions can tell you what globals have been added between the times they were called.
-- Call this when you want to count all current globals as "ignored".
function GlobalsTrackStart()
	gts={};
	i=1;
	for n,v in pairs(_G) do
		gts[i]=n;
		i=i+1;
	end
end
-- Call this when you want to print all non-ignored globals.
-- GlobalsTrackStart should be called at the start of a map script, GlobalsTrackEnd should be called at the end of mapDispose. Well, actually, neither should be called in the finished product. This is just for debugging. After you've verified that your map sets all things to nil when it's done, you should get rid of these function calls.
function GlobalsTrackPrint()
	print("Start printing untracked globals...");
	local got=false;
	for n,v in pairs(_G) do
		for i=1,#gts do
			if (n==gts[i]) then
				got=true;
				break;
			end
		end
		if (got==false) then
			print(n);
		end
		got=false;
	end
	got=nil;
	print("End printing untracked globals...");
end

function GlobalsTrackRemove()
	local got=false;
	for n,v in pairs(_G) do
		for i=1,#gts do
			if (n==gts[i]) then
				got=true;
				break;
			end
		end
		if (got==false) then
			_G[n]=nil;
		end
		got=false;
	end
	got=nil;
end