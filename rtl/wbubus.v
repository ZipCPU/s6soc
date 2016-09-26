////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	wbubus.v
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	This is a test of the Verilog obfuscator routine I put together.
//		The actual code for wbubus.v can be found in the XuLA2-LX25
//	SoC project.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2015-2016, Gisselquist Technology, LLC
//
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of  the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  (It's in the $(ROOT)/doc directory, run make with no
// target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
//
// License:	GPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
//
module wbubus(i_clk,i_rx_stb,i_rx_data,o_wb_cyc,o_wb_stb,o_wb_we,o_wb_addr,
o_wb_data,i_wb_ack,i_wb_stall,i_wb_err,i_wb_data,i_interrupt,o_tx_stb,o_tx_data
,i_tx_busy,o_dbg);parameter LGWATCHDOG=19;input i_clk;input i_rx_stb;input[7:0
]i_rx_data;output wire o_wb_cyc,o_wb_stb,o_wb_we;output wire[31:0]o_wb_addr,
o_wb_data;input i_wb_ack,i_wb_stall,i_wb_err;input[31:0]i_wb_data;input
i_interrupt;output wire o_tx_stb;output wire[7:0]o_tx_data;input i_tx_busy;
output wire o_dbg;reg GUwcb2;wire HUwcb2;wire[35:0]IUwcb2;JUwcb2 KUwcb2(i_clk,
i_rx_stb,i_rx_data,HUwcb2,IUwcb2);wire LUwcb2,MUwcb2,NUwcb2,OUwcb2;wire[35:0]
PUwcb2,QUwcb2;wire RUwcb2,SUwcb2;assign MUwcb2=(~LUwcb2)&&(RUwcb2);assign
OUwcb2=GUwcb2;TUwcb2#(36,6)UUwcb2(i_clk,OUwcb2,HUwcb2,IUwcb2,MUwcb2,PUwcb2,
RUwcb2,SUwcb2);VUwcb2 WUwcb2(i_clk,GUwcb2,MUwcb2,PUwcb2,LUwcb2,o_wb_cyc,
o_wb_stb,o_wb_we,o_wb_addr,o_wb_data,i_wb_ack,i_wb_stall,i_wb_err,i_wb_data,
NUwcb2,QUwcb2);wire XUwcb2;YUwcb2 ZUwcb2(i_clk,OUwcb2,NUwcb2,QUwcb2,o_wb_cyc,
i_interrupt,NUwcb2,o_tx_stb,o_tx_data,i_tx_busy,XUwcb2);reg[(LGWATCHDOG-1):0]
aVwcb2;initial GUwcb2=1'd0;initial aVwcb2=0;always@(posedge i_clk)if((~o_wb_cyc
)||(i_wb_ack))begin aVwcb2<=0;GUwcb2<=1'd0;end else if(&aVwcb2)begin GUwcb2<=
1'd1;aVwcb2<=0;end else begin aVwcb2<=aVwcb2+{{(LGWATCHDOG-1){1'd0}},1'd1};
GUwcb2<=1'd0;end assign o_dbg=OUwcb2;endmodule module bVwcb2(i_clk,cVwcb2,
dVwcb2,eVwcb2,fVwcb2);input i_clk,cVwcb2;input[35:0]dVwcb2;output reg eVwcb2;
output reg[35:0]fVwcb2;wire gVwcb2=(dVwcb2[35:33]==3'd3);reg[7:0]hVwcb2;initial
hVwcb2=8'd0;always@(posedge i_clk)if((cVwcb2)&&(gVwcb2))hVwcb2<=hVwcb2+8'd1;reg
[31:0]iVwcb2[0:255];always@(posedge i_clk)if(cVwcb2)iVwcb2[hVwcb2]<={dVwcb2[32
:31],dVwcb2[29:0]};reg[35:0]jVwcb2;always@(posedge i_clk)if(cVwcb2)jVwcb2<=
dVwcb2;reg[7:0]kVwcb2;always@(posedge i_clk)kVwcb2=hVwcb2-{jVwcb2[32:31],jVwcb2
[29:24]};reg[24:0]lVwcb2;always@(posedge i_clk)case(jVwcb2[32:30])3'd0:lVwcb2
<={19'd0,jVwcb2[29:24]};3'd2:lVwcb2<={13'd0,jVwcb2[29:18]};3'd4:lVwcb2<={7'd0,
jVwcb2[29:12]};3'd6:lVwcb2<={1'd0,jVwcb2[29:6]};3'd1:lVwcb2<={{(19){jVwcb2[29]
}},jVwcb2[29:24]};3'd3:lVwcb2<={{(13){jVwcb2[29]}},jVwcb2[29:18]};3'd5:lVwcb2
<={{(7){jVwcb2[29]}},jVwcb2[29:12]};3'd7:lVwcb2<={{(1){jVwcb2[29]}},jVwcb2[29:6
]};endcase wire[31:0]mVwcb2;assign mVwcb2={{(7){lVwcb2[24]}},lVwcb2};reg[9:0]
nVwcb2;always@(posedge i_clk)if(~jVwcb2[34])nVwcb2<=10'd1+{6'd0,jVwcb2[33:31]}
;else nVwcb2<=10'd9+{1'd0,jVwcb2[33:31],jVwcb2[29:24]};reg[31:0]oVwcb2;always@
(posedge i_clk)oVwcb2<=iVwcb2[kVwcb2];reg[2:0]pVwcb2;initial pVwcb2=0;always@(
posedge i_clk)pVwcb2<={pVwcb2[1:0],cVwcb2};always@(posedge i_clk)eVwcb2<=pVwcb2
[2];always@(posedge i_clk)if(jVwcb2[35:30]==6'd46)fVwcb2<=jVwcb2;else casez(
jVwcb2[35:30])6'b001??0:fVwcb2<={4'd0,mVwcb2[31:0]};6'b001??1:fVwcb2<={3'd1,
mVwcb2[31:30],1'd1,mVwcb2[29:0]};6'b010???:fVwcb2<={3'd3,oVwcb2[31:30],jVwcb2[
30],oVwcb2[29:0]};6'b10????:fVwcb2<={5'd24,jVwcb2[30],20'd0,nVwcb2};6'b11????:
fVwcb2<={5'd24,jVwcb2[30],20'd0,nVwcb2};default:fVwcb2<=jVwcb2;endcase
endmodule module TUwcb2(i_clk,qVwcb2,rVwcb2,sVwcb2,tVwcb2,uVwcb2,vVwcb2,wVwcb2
);parameter BW=66,LGFLEN=10,FLEN=(1<<LGFLEN);input i_clk,qVwcb2;input rVwcb2;
input[(BW-1):0]sVwcb2;input tVwcb2;output reg[(BW-1):0]uVwcb2;output reg vVwcb2
;output wire wVwcb2;reg[(BW-1):0]xVwcb2[0:(FLEN-1)];reg[(LGFLEN-1):0]yVwcb2,
zVwcb2;reg AVwcb2;initial AVwcb2=1'd0;always@(posedge i_clk)if(qVwcb2)AVwcb2<=
1'd0;else if(tVwcb2)AVwcb2<=(AVwcb2)&&(rVwcb2);else if(rVwcb2)AVwcb2<=(yVwcb2+2
==zVwcb2);else if(yVwcb2+1==zVwcb2)AVwcb2<=1'd1;initial yVwcb2=0;always@(
posedge i_clk)if(qVwcb2)yVwcb2<={(LGFLEN){1'd0}};else if(rVwcb2)begin if((
tVwcb2)||(~AVwcb2))yVwcb2<=yVwcb2+{{(LGFLEN-1){1'd0}},1'd1};end always@(posedge
i_clk)if(rVwcb2)xVwcb2[yVwcb2]<=sVwcb2;reg BVwcb2;initial BVwcb2=1'd0;always@(
posedge i_clk)if(qVwcb2)BVwcb2<=1'd0;else if(rVwcb2)BVwcb2<=(BVwcb2)&&(tVwcb2)
;else if(tVwcb2)BVwcb2<=(zVwcb2+1==yVwcb2);else BVwcb2<=(zVwcb2==yVwcb2);
initial zVwcb2=0;always@(posedge i_clk)if(qVwcb2)zVwcb2<={(LGFLEN){1'd0}};else
if(tVwcb2)begin if((rVwcb2)||(~BVwcb2))zVwcb2<=zVwcb2+{{(LGFLEN-1){1'd0}},1'd1
};end always@(posedge i_clk)uVwcb2<=xVwcb2[(tVwcb2)?(zVwcb2+{{(LGFLEN-1){1'd0}
},1'd1}):(zVwcb2)];wire[(LGFLEN-1):0]CVwcb2;assign CVwcb2=yVwcb2+{{(LGFLEN-1){
1'd0}},1'd1};assign wVwcb2=((rVwcb2)&&(AVwcb2)&&(~tVwcb2))||((tVwcb2)&&(BVwcb2
)&&(~rVwcb2));wire[(LGFLEN-1):0]DVwcb2;assign DVwcb2=zVwcb2+{{(LGFLEN-1){1'd0}
},1'd1};always@(posedge i_clk)if(qVwcb2)vVwcb2<=1'd0;else vVwcb2<=(~tVwcb2)&&(
yVwcb2!=zVwcb2)||(tVwcb2)&&(yVwcb2!=DVwcb2);endmodule module YUwcb2(i_clk,
qVwcb2,cVwcb2,EVwcb2,FVwcb2,GVwcb2,HVwcb2,eVwcb2,IVwcb2,i_tx_busy,JVwcb2);input
i_clk,qVwcb2;input cVwcb2;input[35:0]EVwcb2;input FVwcb2,GVwcb2,HVwcb2;output
wire eVwcb2;output wire[7:0]IVwcb2;input i_tx_busy;output wire JVwcb2;wire
KVwcb2,LVwcb2,MVwcb2,NVwcb2;wire[35:0]OVwcb2;wire PVwcb2,QVwcb2,RVwcb2,SVwcb2,
TVwcb2,UVwcb2,VVwcb2,WVwcb2;wire[35:0]XVwcb2,YVwcb2;wire[6:0]ZVwcb2,aWwcb2;
assign KVwcb2=(MVwcb2)&&(~QVwcb2);TUwcb2#(36,10)bWwcb2(i_clk,qVwcb2,cVwcb2,
EVwcb2,KVwcb2,OVwcb2,MVwcb2,NVwcb2);assign JVwcb2=NVwcb2;cWwcb2 dWwcb2(i_clk,
KVwcb2,OVwcb2,FVwcb2,HVwcb2,GVwcb2,PVwcb2,XVwcb2,QVwcb2,VVwcb2);assign VVwcb2=
RVwcb2;eWwcb2 fWwcb2(i_clk,PVwcb2,XVwcb2,RVwcb2,YVwcb2,LVwcb2);gWwcb2 hWwcb2(
i_clk,RVwcb2,YVwcb2,UVwcb2,SVwcb2,ZVwcb2,LVwcb2);iWwcb2 jWwcb2(i_clk,SVwcb2,
ZVwcb2,TVwcb2,aWwcb2,(FVwcb2||HVwcb2||MVwcb2||QVwcb2),WVwcb2,UVwcb2);kWwcb2
lWwcb2(i_clk,TVwcb2,aWwcb2,eVwcb2,IVwcb2,WVwcb2,i_tx_busy);endmodule module
mWwcb2(i_clk,cVwcb2,nWwcb2,eVwcb2,oWwcb2,pWwcb2);input i_clk,cVwcb2;input[7:0]
nWwcb2;output reg eVwcb2,oWwcb2;output reg[5:0]pWwcb2;always@(posedge i_clk)
eVwcb2<=cVwcb2;always@(posedge i_clk)begin oWwcb2<=1'd1;pWwcb2<=6'd0;if((nWwcb2
>=8'd48)&&(nWwcb2<=8'd57))pWwcb2<={2'd0,nWwcb2[3:0]};else if((nWwcb2>=8'd65)&&
(nWwcb2<=8'd90))pWwcb2<=(nWwcb2[5:0]-6'd1+6'd10);else if((nWwcb2>=8'd97)&&(
nWwcb2<=8'd122))pWwcb2<=(nWwcb2[5:0]+6'd3);else if(nWwcb2==8'd64)pWwcb2<=6'd62
;else if(nWwcb2==8'd37)pWwcb2<=6'd63;else oWwcb2<=1'd0;end endmodule module
iWwcb2(i_clk,cVwcb2,qWwcb2,eVwcb2,rWwcb2,HVwcb2,i_tx_busy,sWwcb2);input i_clk,
cVwcb2;input[6:0]qWwcb2;output reg eVwcb2;output reg[6:0]rWwcb2;input HVwcb2;
input i_tx_busy;output wire sWwcb2;reg tWwcb2,uWwcb2;initial tWwcb2=1'd1;
initial uWwcb2=1'd1;always@(posedge i_clk)if((~i_tx_busy)&&(eVwcb2))tWwcb2<=(
rWwcb2[6]);always@(posedge i_clk)if((cVwcb2)&&(~sWwcb2))uWwcb2<=(qWwcb2[6]);reg
[6:0]vWwcb2;initial vWwcb2=7'd0;always@(posedge i_clk)if((~i_tx_busy)&&(eVwcb2
))begin if(rWwcb2[6])vWwcb2<=0;else vWwcb2<=vWwcb2+7'd1;end reg wWwcb2;initial
wWwcb2=1'd0;always@(posedge i_clk)wWwcb2<=(vWwcb2>7'd72);initial eVwcb2=1'd0;
always@(posedge i_clk)if((cVwcb2)&&(~sWwcb2))begin eVwcb2<=(wWwcb2)||(~qWwcb2[6
]);rWwcb2<=qWwcb2;end else if(~sWwcb2)begin eVwcb2<=(~i_tx_busy)&&(~HVwcb2)&&(
~tWwcb2)&&(uWwcb2);rWwcb2<=7'd64;end else if(~i_tx_busy)eVwcb2<=1'd0;reg xWwcb2
;initial xWwcb2=1'd0;always@(posedge i_clk)xWwcb2<=(eVwcb2);assign sWwcb2=(
xWwcb2)||(eVwcb2);endmodule module gWwcb2(i_clk,cVwcb2,dVwcb2,i_tx_busy,eVwcb2
,rWwcb2,sWwcb2);input i_clk,cVwcb2;input[35:0]dVwcb2;input i_tx_busy;output reg
eVwcb2;output reg[6:0]rWwcb2;output reg sWwcb2;wire[2:0]yWwcb2;assign yWwcb2=(
dVwcb2[35:33]==3'd0)?3'd1:(dVwcb2[35:32]==4'd2)?3'd6:(dVwcb2[35:32]==4'd3)?(
3'd2+{1'd0,dVwcb2[31:30]}):(dVwcb2[35:34]==2'd1)?3'd2:(dVwcb2[35:34]==2'd2)?
3'd1:3'd6;reg zWwcb2;reg[2:0]AWwcb2;reg[29:0]jVwcb2;initial eVwcb2=1'd0;initial
sWwcb2=1'd0;initial zWwcb2=1'd0;always@(posedge i_clk)if((cVwcb2)&&(~sWwcb2))
begin AWwcb2<=yWwcb2-3'd1;jVwcb2<=dVwcb2[29:0];eVwcb2<=1'd1;rWwcb2<={1'd0,
dVwcb2[35:30]};sWwcb2<=1'd1;zWwcb2<=1'd1;end else if((eVwcb2)&&(i_tx_busy))
begin sWwcb2<=1'd1;zWwcb2<=1'd1;end else if(eVwcb2)eVwcb2<=1'd0;else if(AWwcb2
>0)begin eVwcb2<=1'd1;rWwcb2<={1'd0,jVwcb2[29:24]};jVwcb2[29:6]<=jVwcb2[23:0];
AWwcb2<=AWwcb2-3'd1;sWwcb2<=1'd1;zWwcb2<=1'd1;end else if(~rWwcb2[6])begin
eVwcb2<=1'd1;rWwcb2<=7'd64;sWwcb2<=1'd1;zWwcb2<=1'd1;end else begin zWwcb2<=
1'd0;sWwcb2<=(zWwcb2);end endmodule module cWwcb2(i_clk,cVwcb2,EVwcb2,BWwcb2,
CWwcb2,GVwcb2,eVwcb2,DWwcb2,sWwcb2,i_tx_busy);input i_clk;input cVwcb2;input[35
:0]EVwcb2;input BWwcb2,CWwcb2,GVwcb2;output reg eVwcb2;output reg[35:0]DWwcb2;
output reg sWwcb2;input i_tx_busy;reg EWwcb2,FWwcb2;initial EWwcb2=1'd0;always
@(posedge i_clk)if((eVwcb2)&&(~i_tx_busy)&&(DWwcb2[35:30]==6'd4))EWwcb2<=GVwcb2
;else EWwcb2<=(EWwcb2)||(GVwcb2);wire GWwcb2;reg HWwcb2;reg[35:0]IWwcb2;initial
IWwcb2=36'd0;always@(posedge i_clk)if((cVwcb2)||(eVwcb2))IWwcb2<=36'd0;else if
(~IWwcb2[35])IWwcb2<=IWwcb2+36'd43;initial HWwcb2=1'd0;always@(posedge i_clk)if
((eVwcb2)&&(~i_tx_busy)&&(DWwcb2[35:31]==5'd0))HWwcb2<=1'd1;else if(~IWwcb2[35
])HWwcb2<=1'd0;assign GWwcb2=(~HWwcb2)&&(IWwcb2[35]);initial eVwcb2=1'd0;
initial sWwcb2=1'd0;always@(posedge i_clk)if((eVwcb2)&&(i_tx_busy))begin sWwcb2
<=1'd1;end else if(eVwcb2)begin eVwcb2<=1'd0;sWwcb2<=1'd1;end else if(sWwcb2)
sWwcb2<=1'd0;else if(cVwcb2)begin DWwcb2<=EVwcb2;eVwcb2<=1'd1;sWwcb2<=1'd1;end
else if((EWwcb2)&&(~FWwcb2))begin eVwcb2<=1'd1;DWwcb2<={6'd4,30'd0};sWwcb2<=
1'd1;end else if(GWwcb2)begin eVwcb2<=1'd1;sWwcb2<=1'd1;if(BWwcb2)DWwcb2<={6'd1
,30'd0};else DWwcb2<={6'd0,30'd0};end initial FWwcb2=1'd0;always@(posedge i_clk
)if((EWwcb2)&&((~eVwcb2)&&(~sWwcb2)&&(~cVwcb2)))FWwcb2<=1'd1;else if(~GVwcb2)
FWwcb2<=1'd0;endmodule module JWwcb2(i_clk,cVwcb2,KWwcb2,LWwcb2,eVwcb2,DWwcb2)
;input i_clk,cVwcb2,KWwcb2;input[5:0]LWwcb2;output reg eVwcb2;output reg[35:0]
DWwcb2;reg[2:0]AWwcb2,MWwcb2;wire NWwcb2;assign NWwcb2=((AWwcb2==MWwcb2)&&(
MWwcb2!=0))||((cVwcb2)&&(~KWwcb2)&&(OWwcb2==2'd1));initial AWwcb2=3'd0;always@
(posedge i_clk)if((cVwcb2)&&(~KWwcb2))AWwcb2<=0;else if(NWwcb2)AWwcb2<=(cVwcb2
)?3'd1:3'd0;else if(cVwcb2)AWwcb2<=AWwcb2+3'd1;reg[35:0]PWwcb2;always@(posedge
i_clk)if(NWwcb2)PWwcb2[35:30]<=LWwcb2;else if(cVwcb2)case(AWwcb2)3'd0:PWwcb2[35
:30]<=LWwcb2;3'd1:PWwcb2[29:24]<=LWwcb2;3'd2:PWwcb2[23:18]<=LWwcb2;3'd3:PWwcb2
[17:12]<=LWwcb2;3'd4:PWwcb2[11:6]<=LWwcb2;3'd5:PWwcb2[5:0]<=LWwcb2;default:
begin end endcase reg[1:0]OWwcb2;always@(posedge i_clk)if(eVwcb2)OWwcb2<=DWwcb2
[35:34];always@(posedge i_clk)if((cVwcb2)&&(~KWwcb2)&&(OWwcb2==2'd1))DWwcb2[35
:30]<=6'd46;else DWwcb2<=PWwcb2;initial MWwcb2=3'd0;always@(posedge i_clk)if((
cVwcb2)&&(~KWwcb2))MWwcb2<=0;else if((cVwcb2)&&((MWwcb2==0)||(NWwcb2)))begin if
(LWwcb2[5:4]==2'd3)MWwcb2<=3'd2;else if(LWwcb2[5:4]==2'd2)MWwcb2<=3'd1;else if
(LWwcb2[5:3]==3'd2)MWwcb2<=3'd2;else if(LWwcb2[5:3]==3'd1)MWwcb2<=3'd2+{1'd0,
LWwcb2[2:1]};else MWwcb2<=3'd6;end else if(NWwcb2)MWwcb2<=0;always@(posedge
i_clk)eVwcb2<=NWwcb2;endmodule module eWwcb2(i_clk,cVwcb2,EVwcb2,eVwcb2,QWwcb2
,CWwcb2);parameter DW=32,CW=36,TBITS=10;input i_clk,cVwcb2;input[(CW-1):0]
EVwcb2;output wire eVwcb2;output wire[(CW-1):0]QWwcb2;input CWwcb2;reg RWwcb2;
reg[35:0]SWwcb2;wire[31:0]mVwcb2;assign mVwcb2=EVwcb2[31:0];always@(posedge
i_clk)if((cVwcb2)&&(~RWwcb2))begin if(EVwcb2[35:32]!=4'd2)begin SWwcb2<=EVwcb2
;end else if(mVwcb2[31:6]==26'd0)SWwcb2<={6'd12,mVwcb2[5:0],24'd0};else if(
mVwcb2[31:12]==20'd0)SWwcb2<={6'd13,mVwcb2[11:0],18'd0};else if(mVwcb2[31:18]
==14'd0)SWwcb2<={6'd14,mVwcb2[17:0],12'd0};else if(mVwcb2[31:24]==8'd0)SWwcb2
<={6'd15,mVwcb2[23:0],6'd0};else begin SWwcb2<=EVwcb2;end end initial RWwcb2=
1'd0;always@(posedge i_clk)if((cVwcb2)&&(~RWwcb2))RWwcb2<=cVwcb2;else if(~
CWwcb2)RWwcb2<=1'd0;wire TWwcb2;assign TWwcb2=(RWwcb2)&&(~CWwcb2);reg pVwcb2;
always@(posedge i_clk)pVwcb2<=RWwcb2;wire[35:0]jVwcb2;assign jVwcb2=SWwcb2;reg
[(TBITS-1):0]UWwcb2;reg VWwcb2,WWwcb2;always@(posedge i_clk)if(TWwcb2)begin if
(QWwcb2[35:33]==3'd1)UWwcb2<=0;else if(QWwcb2[35:33]==3'd7)UWwcb2<=UWwcb2+{{(
TBITS-1){1'd0}},1'd1};end always@(posedge i_clk)if((TWwcb2)&&(QWwcb2[35:33]==
3'd1))WWwcb2<=1'd0;else if(UWwcb2==10'd1023)WWwcb2<=1'd1;reg[31:0]iVwcb2[0:((1
<<TBITS)-1)];always@(posedge i_clk)iVwcb2[UWwcb2]<={jVwcb2[32:31],jVwcb2[29:0]
};reg XWwcb2,YWwcb2;reg[(TBITS-1):0]ZWwcb2;reg[(TBITS-1):0]aXwcb2;initial
ZWwcb2=0;initial XWwcb2=0;always@(posedge i_clk)begin YWwcb2<=((aXwcb2-UWwcb2)
=={{(TBITS-1){1'd0}},1'd1});if((TWwcb2)||(~RWwcb2))begin ZWwcb2<=UWwcb2+{(TBITS
){1'd1}};aXwcb2=UWwcb2+{{(TBITS-1){1'd1}},1'd0};XWwcb2<=1'd0;end else if((~
XWwcb2)&&(~bXwcb2)&&((~aXwcb2[TBITS-1])||(WWwcb2)))begin ZWwcb2<=aXwcb2;aXwcb2
=aXwcb2-{{(TBITS-1){1'd0}},1'd1};XWwcb2<=YWwcb2;end end reg[1:0]cXwcb2;reg
dXwcb2,eXwcb2;reg[(DW-1):0]oVwcb2;reg[(TBITS-1):0]fXwcb2,gXwcb2,hXwcb2;always@
(posedge i_clk)begin oVwcb2<=iVwcb2[ZWwcb2];fXwcb2<=ZWwcb2;dXwcb2<=(oVwcb2=={
jVwcb2[32:31],jVwcb2[29:0]});gXwcb2<=fXwcb2;hXwcb2<=UWwcb2-fXwcb2;eXwcb2<=({
1'd0,fXwcb2}<{WWwcb2,UWwcb2})&&(fXwcb2!=UWwcb2);end always@(posedge i_clk)if((
TWwcb2)||(~RWwcb2))cXwcb2<=0;else cXwcb2<={cXwcb2[0],1'd1};reg bXwcb2;reg[9:0]
iXwcb2;always@(posedge i_clk)if((TWwcb2)||(~RWwcb2)||(~pVwcb2))bXwcb2<=1'd0;
else if(~bXwcb2)begin bXwcb2<=(eXwcb2)&&(dXwcb2)&&(jVwcb2[35:33]==3'd7)&&(
cXwcb2==2'd3);end reg jXwcb2,kXwcb2,lXwcb2;always@(posedge i_clk)if(~bXwcb2)
begin iXwcb2<=hXwcb2;lXwcb2<=(hXwcb2<10'd1313);jXwcb2<=(hXwcb2==10'd1);kXwcb2
<=(hXwcb2<10'd10);end wire[(TBITS-1):0]mXwcb2;wire[9:0]nXwcb2;wire[2:0]oXwcb2;
assign mXwcb2=iXwcb2;assign oXwcb2=iXwcb2[2:0]-3'd2;assign nXwcb2=iXwcb2-10'd10
;initial VWwcb2=1'd0;reg[(CW-1):0]pXwcb2;always@(posedge i_clk)begin if((~
RWwcb2)||(~pVwcb2)||(TWwcb2))begin pXwcb2<=jVwcb2;VWwcb2<=1'd0;end else if((
bXwcb2)&&(lXwcb2))begin pXwcb2<=jVwcb2;if(jXwcb2)pXwcb2[35:30]<={5'd3,jVwcb2[30
]};else if(kXwcb2)pXwcb2[35:30]<={2'd2,oXwcb2,jVwcb2[30]};else pXwcb2[35:24]<=
{2'd1,nXwcb2[8:6],jVwcb2[30],nXwcb2[5:0]};end else pXwcb2<=jVwcb2;end assign
eVwcb2=RWwcb2;assign QWwcb2=(pVwcb2)?(pXwcb2):(SWwcb2);endmodule module VUwcb2
(i_clk,qVwcb2,cVwcb2,EVwcb2,sWwcb2,o_wb_cyc,o_wb_stb,o_wb_we,o_wb_addr,
o_wb_data,i_wb_ack,i_wb_stall,i_wb_err,i_wb_data,eVwcb2,DWwcb2);input i_clk,
qVwcb2;input cVwcb2;input[35:0]EVwcb2;output wire sWwcb2;output reg o_wb_cyc;
output reg o_wb_stb;output reg o_wb_we;output reg[31:0]o_wb_addr,o_wb_data;
input i_wb_ack,i_wb_stall,i_wb_err;input[31:0]i_wb_data;output reg eVwcb2;
output reg[35:0]DWwcb2;wire qXwcb2,rXwcb2,sXwcb2,tXwcb2;assign qXwcb2=(cVwcb2)
&&(~sWwcb2);assign sXwcb2=(qXwcb2)&&(EVwcb2[35:34]==2'd1);assign rXwcb2=(qXwcb2
)&&(EVwcb2[35:30]==6'd46);wire[31:0]uXwcb2;assign uXwcb2={EVwcb2[32:31],EVwcb2
[29:0]};assign tXwcb2=((qXwcb2)&&(EVwcb2[35:33]!=3'd3)&&(EVwcb2[35:30]!=6'd46)
);reg[2:0]vXwcb2;reg[9:0]wXwcb2,AWwcb2;reg xXwcb2,yXwcb2,zXwcb2,AXwcb2,BXwcb2;
reg CXwcb2;initial yXwcb2=1'd1;initial vXwcb2=3'd0;initial eVwcb2=1'd0;always@
(posedge i_clk)if(qVwcb2)begin vXwcb2<=3'd0;eVwcb2<=1'd1;DWwcb2<={6'd3,
i_wb_data[29:0]};o_wb_cyc<=1'd0;o_wb_stb<=1'd0;end else case(vXwcb2)3'd0:begin
o_wb_cyc<=1'd0;o_wb_stb<=1'd0;eVwcb2<=1'd0;xXwcb2<=EVwcb2[30];o_wb_we<=(~EVwcb2
[35]);DWwcb2<={4'd2,o_wb_addr};o_wb_we<=(EVwcb2[35:34]!=2'd3);o_wb_data<=uXwcb2
;if(cVwcb2)begin casez(EVwcb2[35:32])4'd0:begin o_wb_addr<=EVwcb2[31:0];end
4'b001?:begin o_wb_addr<=o_wb_addr+{EVwcb2[32:31],EVwcb2[29:0]};end 4'b01??:
begin vXwcb2<=3'd2;o_wb_cyc<=1'd1;o_wb_stb<=1'd1;end 4'b11??:begin if(yXwcb2)
eVwcb2<=1'd1;vXwcb2<=3'd1;o_wb_cyc<=1'd1;o_wb_stb<=1'd1;end default:;endcase
end end 3'd1:begin o_wb_cyc<=1'd1;o_wb_stb<=1'd1;if(i_wb_err)vXwcb2<=3'd0;
eVwcb2<=(i_wb_err)||(i_wb_ack);if(i_wb_err)DWwcb2<={6'd5,i_wb_data[29:0]};else
DWwcb2<={3'd7,i_wb_data[31:30],xXwcb2,i_wb_data[29:0]};if((xXwcb2)&&(~
i_wb_stall))o_wb_addr<=o_wb_addr+32'd1;if(~i_wb_stall)begin if((CXwcb2)||(
zXwcb2))begin vXwcb2<=3'd3;o_wb_stb<=1'd0;end end end 3'd2:begin o_wb_cyc<=1'd1
;o_wb_stb<=1'd1;if(i_wb_err)DWwcb2<={6'd5,i_wb_data[29:0]};else DWwcb2<={6'd2,
i_wb_data[29:0]};if((xXwcb2)&&(~i_wb_stall))o_wb_addr<=o_wb_addr+32'd1;eVwcb2
<=1'd1;if(i_wb_err)begin vXwcb2<=3'd5;o_wb_cyc<=1'd0;o_wb_stb<=1'd0;end else if
(~i_wb_stall)begin vXwcb2<=3'd4;o_wb_stb<=1'd0;end end 3'd3:begin o_wb_cyc<=
1'd1;o_wb_stb<=1'd0;if(i_wb_err)DWwcb2<={6'd5,i_wb_data[29:0]};else DWwcb2<={
3'd7,i_wb_data[31:30],xXwcb2,i_wb_data[29:0]};eVwcb2<=(((i_wb_ack)&&(~o_wb_we)
)||(i_wb_err));if(((AXwcb2)&&(i_wb_ack))||(BXwcb2)||(i_wb_err))begin o_wb_cyc
<=1'd0;vXwcb2<=3'd0;end end 3'd4:begin DWwcb2<={6'd5,i_wb_data[29:0]};eVwcb2<=
(i_wb_err)||(tXwcb2);o_wb_data<=uXwcb2;o_wb_cyc<=1'd1;o_wb_stb<=1'd0;if(tXwcb2
)begin o_wb_cyc<=1'd0;vXwcb2<=3'd0;end else if(i_wb_err)begin o_wb_cyc<=1'd0;
vXwcb2<=3'd5;end else if(sXwcb2)begin vXwcb2<=3'd2;o_wb_stb<=1'd1;end else if(
rXwcb2)vXwcb2<=3'd3;end 3'd5:begin o_wb_cyc<=1'd0;o_wb_stb<=1'd0;DWwcb2<={6'd5
,i_wb_data[29:0]};eVwcb2<=(tXwcb2);if((rXwcb2)||(tXwcb2))vXwcb2<=3'd0;end
default:begin eVwcb2<=1'd1;DWwcb2<={6'd3,i_wb_data[29:0]};vXwcb2<=3'd0;o_wb_cyc
<=1'd0;o_wb_stb<=1'd0;end endcase assign sWwcb2=(vXwcb2!=3'd0)&&(vXwcb2!=3'd4)
&&(vXwcb2!=3'd5);always@(posedge i_clk)if(qVwcb2)yXwcb2<=1'd1;else if((~
o_wb_cyc)&&(cVwcb2)&&(~EVwcb2[35]))yXwcb2<=1'd1;else if(o_wb_cyc)yXwcb2<=1'd0;
always@(posedge i_clk)if(~o_wb_cyc)wXwcb2<=10'd0;else if((o_wb_stb)&&(~
i_wb_stall)&&(~i_wb_ack))wXwcb2<=wXwcb2+10'd1;else if(((~o_wb_stb)||(i_wb_stall
))&&(i_wb_ack))wXwcb2<=wXwcb2-10'd1;always@(posedge i_clk)AXwcb2<=(~o_wb_stb)
&&(wXwcb2==10'd1)||(o_wb_stb)&&(wXwcb2==10'd0);always@(posedge i_clk)BXwcb2<=(
~o_wb_stb)&&(wXwcb2==10'd0);always@(posedge i_clk)if(~o_wb_cyc)AWwcb2<=EVwcb2[9
:0];else if((o_wb_stb)&&(~i_wb_stall)&&(|AWwcb2))AWwcb2<=AWwcb2-10'd1;always@(
posedge i_clk)begin CXwcb2<=(~o_wb_cyc)&&(EVwcb2[9:0]==10'd1);zXwcb2<=(o_wb_stb
)&&(AWwcb2[9:2]==8'd0)&&((~AWwcb2[1])||((~AWwcb2[0])&&(~i_wb_stall)));end
endmodule module JUwcb2(i_clk,cVwcb2,nWwcb2,eVwcb2,DWwcb2);input i_clk,cVwcb2;
input[7:0]nWwcb2;output wire eVwcb2;output wire[35:0]DWwcb2;wire DXwcb2,EXwcb2
;wire[5:0]FXwcb2;mWwcb2 GXwcb2(i_clk,cVwcb2,nWwcb2,DXwcb2,EXwcb2,FXwcb2);wire
PVwcb2;wire[35:0]HXwcb2;JWwcb2 IXwcb2(i_clk,DXwcb2,EXwcb2,FXwcb2,PVwcb2,HXwcb2
);bVwcb2 JXwcb2(i_clk,PVwcb2,HXwcb2,eVwcb2,DWwcb2);endmodule module kWwcb2(
i_clk,cVwcb2,KXwcb2,eVwcb2,IVwcb2,sWwcb2,CWwcb2);input i_clk;input cVwcb2;input
[6:0]KXwcb2;output reg eVwcb2;output reg[7:0]IVwcb2;output wire sWwcb2;input
CWwcb2;initial IVwcb2=8'd0;always@(posedge i_clk)if((cVwcb2)&&(~sWwcb2))begin
if(KXwcb2[6])IVwcb2<=8'd10;else if(KXwcb2[5:0]<=6'd9)IVwcb2<=8'd48+{4'd0,KXwcb2
[3:0]};else if(KXwcb2[5:0]<=6'd35)IVwcb2<=8'd65+{2'd0,KXwcb2[5:0]}-8'd10;else
if(KXwcb2[5:0]<=6'd61)IVwcb2<=8'd97+{2'd0,KXwcb2[5:0]}-8'd36;else if(KXwcb2[5:0
]==6'd62)IVwcb2<=8'd64;else IVwcb2<=8'd37;end always@(posedge i_clk)if((eVwcb2
)&&(~CWwcb2))eVwcb2<=1'd0;else if((cVwcb2)&&(~eVwcb2))eVwcb2<=1'd1;assign
sWwcb2=eVwcb2;endmodule
