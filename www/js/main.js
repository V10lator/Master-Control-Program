var serverTime = 0
var clockInterval = null;
var siteCleanup = null;

$(function()
{
	clockTick();

	var menu = $("#menu");
	var mobile_menu = $("#mobile_menu");

	var menu_items = ["Home", "Test1", "Test2", "Test3", "Test4", "Test5"];
	menu_items.forEach(function(val)
	{
		menu.append("<button type=\"button\" class=\"btn btn-outline-info\" onclick=\"menu_navigate('" + val + "')\">" + val + "</button>");
		mobile_menu.append("<li><a class=\"dropdown-item\" href=\"#\" onclick=\"menu_navigate('" + val + "')\">" + val + "</a></li>");
	});

	menu_navigate("Home");
});

function toggleActive(item, active)
{
	if(item.text().normalize() === active)
		item.addClass("active");
	else
		item.removeClass("active");
}

function menu_navigate(entry)
{
	if(siteCleanup != null)
	{
		siteCleanup();
		siteCleanup = null;
	}

	$("#main").load("/templates/" + entry + ".html", null, function() {
		$.getScript("/js/" + entry + ".js");
	});

	entry = entry.normalize();
	$("#menu").children().each(function() { toggleActive($(this), entry); });
	$("#mobile_menu").children().each(function() { toggleActive($(this).children(), entry); });
}

function addLeadingZero(num)
{
	if(num < 10)
		return "0" + num;
	return num;
}

function _clockTick()
{
	setTimeout(clockTick, 1000);
	var date = new Date(serverTime++ * 1000);
	$("#clock").text(addLeadingZero(date.getDate()) + "." + addLeadingZero(date.getMonth() + 1) + "." + date.getFullYear() + " " + addLeadingZero(date.getHours()) + ":" + addLeadingZero(date.getMinutes()) + ":" + addLeadingZero(date.getSeconds()));
}

function clockTick()
{
	if(serverTime % 60 == 0)
	{
		$.ajax("/api/t").done(function(msg)
		{
			serverTime = msg;
			_clockTick();
		});
		return;
	}

	_clockTick();
}
