/* This function generates menus and search box in narrow/slim fit mode */
var narrowInit = function() {
  /* 1: Create search form */ 
  var narrowSearch = $('<div id="narrowsearch"></div>');
  var searchform = $("#qtdocsearch");
  narrowSearch.append(searchform);
  $("#qtdocheader .content .qtref").after(narrowSearch);

  /* 2: Create dropdowns */ 
  var narrowmenu = $('<ul id="narrowmenu" class="sf-menu"></ul>');

  /* Lookup */ 
  var lookuptext = $("#lookup h2").attr("title");
  $("#lookup ul").removeAttr("id");
  $("#lookup ul li").removeAttr("class");
  $("#lookup ul li").removeAttr("style");
  var lookupul = $("#lookup ul");
  var lookuplist = $('<li></li>');
  var lookuplink = $('<a href="#"></a>');
  lookuplink.append(lookuptext);
  lookuplist.append(lookuplink);
  lookuplist.append(lookupul);
  narrowmenu.append(lookuplist);

  /* Topics */ 
  var topicstext = $("#topics h2").attr("title");
  $("#topics ul").removeAttr("id");
  $("#topics ul li").removeAttr("class");
  $("#topics ul li").removeAttr("style");
  var topicsul = $("#topics ul");
  var topicslist = $('<li></li>');
  var topicslink = $('<a href="#"></a>');
  topicslink.append(topicstext);
  topicslist.append(topicslink);
  topicslist.append(topicsul);
  narrowmenu.append(topicslist);

  /* Examples */ 
  var examplestext = $("#examples h2").attr("title");
  $("#examples ul").removeAttr("id");
  $("#examples ul li").removeAttr("class");
  $("#examples ul li").removeAttr("style");
  var examplesul = $("#examples ul");
  var exampleslist = $('<li></li>');
  var exampleslink = $('<a href="#"></a>');
  exampleslink.append(examplestext);
  exampleslist.append(exampleslink);
  exampleslist.append(examplesul);
  narrowmenu.append(exampleslist);

  $("#shortCut").after(narrowmenu);
  $('ul#narrowmenu').superfish({
    delay: 100,
    autoArrows: false,
    disableHI: true
  });
}

/* Executes on doc ready */
$(document).ready(function(){
	/* check if body has the narrow class */
	if ($('body').hasClass('narrow')) {
		/* run narrowInit */
		narrowInit();
	}
 
	/* messure window width and add class if it is smaller than 600 px */
	if($(window).width()<600) {
		$('body').addClass('narrow');
		/* if the search box contains */
		if ($("#narrowsearch").length == 0) {
			/* run narrowInit */
			narrowInit();
		}
	  }
	  else { /* if the window is wider than 600 px, narrow is removed */
		$('body').removeClass('narrow');
		if ($("#narrowsearch").length == 0) {
		}
	}
});
/* binding resize event to this funciton */
$(window).bind('resize', function () {
	/*  if the window is wider than 600 px, narrow class is added */
	if($(window).width()<600) {
		$('body').addClass('narrow');
		if ($("#narrowsearch").length == 0) {
		  narrowInit();
		}
	}
	else {
		/* else we remove the narrow class */
		$('body').removeClass('narrow');
  }
});

	$('#narrowsearch').keyup(function () {
		/* extract the search box content */
	  var searchString = $('#narrowsearch').val();
	  /* if the string is less than three characters */
	  if ((searchString == null) || (searchString.length < 3)) {
			/* remove classes and elements*/
			$('#narrowsearch').removeClass('loading');
			$('.searching').remove(); 
			/*  run CheckEmptyAndLoadList */
			CheckEmptyAndLoadList();
			
			$('.report').remove();
			return;
	   }
	   /* if timer checks out */
		if (this.timer) clearTimeout(this.timer);
		this.timer = setTimeout(function () {
			/* add loading image by adding loading class */
			$('#narrowsearch').addClass('loading');
			$('.searching').remove(); 

			/* run the actual search */
		   $.ajax({
			contentType: "application/x-www-form-urlencoded",
			url: 'http://' + location.host + '/nokiasearch/GetDataServlet',
			data: 'searchString='+searchString,
			dataType:'xml',
			type: 'post',	 
			success: function (response, textStatus) {
				/* on success remove loading img */
				$('.searching').remove(); 
				$('#narrowsearch').removeClass('loading');
				processNokiaData(response);
			}     
		  });
		}, 500); /* timer set to 500 ms */
	});