// Removing search results
function hideSearchResults() {
/* hiding search results as the user clicks on the different categories */
  $('#resultdialog').removeClass('active');
	$("#resultlist").removeClass().addClass('all');
	$("#resultlinks").removeClass().addClass('all');
	$("#searchcount").removeClass().addClass('all');
}
/* closing the searhc result dialog */
$('#resultclose').click(function(e) {
  e.preventDefault();
  hideSearchResults();
});

$(document.body).click(function() {
});

/* START non link areas where cursor should change to pointing hand */
$('.t_button').mouseover(function() {
    $('.t_button').css('cursor','pointer');
});
/* END non link areas  */
/* Changing font size to smaller */
$('#smallA').click(function() {
		$('.mainContent .heading,.mainContent h1, .mainContent h2, .mainContent h3, .mainContent p, .mainContent li, .mainContent table').css('font-size','smaller');
		$('.t_button').removeClass('active')
		$(this).addClass('active')
});

/* Reset font size */
$('#medA').click(function() {
		$('.mainContent .heading').css('font','600 16px/1 Arial');
		$('.mainContent h1').css('font','600 18px/1.2 Arial');
		$('.mainContent h2').css('font','600 16px/1.2 Arial');
		$('.mainContent h3').css('font','600 14px/1.2 Arial');
		$('.mainContent p').css('font','13px/20px Verdana');
		$('.mainContent li').css('font','400 13px/1 Verdana');
		$('.mainContent li').css('line-height','14px');
		$('.mainContent .toc li').css('font', 'normal 10px/1.2 Verdana');
		$('.mainContent table').css('font','13px/1.2 Verdana');
		$('.mainContent .heading').css('font','600 16px/1 Arial');
		$('.mainContent .indexboxcont li').css('font','600 13px/1 Verdana');
		$('.t_button').removeClass('active')
		$(this).addClass('active')
});
/* Changing font size to bigger */
$('#bigA').click(function() {
		$('.mainContent .heading,.mainContent h1, .mainContent h2, .mainContent h3, .mainContent p, .mainContent li, .mainContent table').css('font-size','large');
		$('.mainContent .heading,.mainContent h1, .mainContent h2, .mainContent h3, .mainContent p, .mainContent li, .mainContent table').css('line-height','25px');
		$('.t_button').removeClass('active')
		$(this).addClass('active')
});

/* Show page content after closing feedback box */
$('.feedclose').click(function() {
	$('.bd').show();
	$('.hd').show();
	$('.footer').show();
	$('#feedbackBox').hide();
	$('#blurpage').hide();
});

/* Hide page content and show feedback box */
$('.feedback').click(function() {
	$('.bd').hide();
	$('.hd').hide();
	$('.footer').hide();
	$('#feedbackBox').show();
	$('#blurpage').show();
});
/* Default search URL */
var qturl = "";

/* The next function handles the response data (in xml) returned by the search engine */

// Process data sent back from the server. The data is structured as a XML.
/*
XML structure handled by function processNokiaData()
<page> - container for each page returned
<pageWords/> - contains keywords
<pageTitle/> - contains page title/header content 
<pageUrl/> - contains page URL - URL relative to root
<pageType> - contains page type - APIPage/Article/Example
</page>
*/


function processNokiaData(response){
/* fetch the responce from the server using page as the root element */
	var propertyTags = response.getElementsByTagName('page');
	/* reset counters */	
	var apiCount = 0;
	var articleCount = 0;
	var exampleCount = 0;
	var full_li_element;

/* remove any old results */
	$('#resultlist li').remove();


	/* running through the elements in the xml structure */
 	for (var i=0; i<propertyTags.length; i++) {
		/* for every element named pageWords*/
		for (var j=0; j< propertyTags[i].getElementsByTagName('pageWords').length; j++) {
			/* start a new list element */
			full_li_element = '<li';
					/* if the pageType element reads APIPage, add class name api */
      if (propertyTags[i].getElementsByTagName('pageType')[0].firstChild.nodeValue == 'APIPage') {
      	full_li_element += ' class="api"';
      	apiCount++;
      }
					/* if the pageType element reads Article, add class name article */
      else if (propertyTags[i].getElementsByTagName('pageType')[0].firstChild.nodeValue == 'Article') {
      	full_li_element += ' class="article"';
      	articleCount++;
      }
					/* if the pageType element reads Example, add class name example */
      else if (propertyTags[i].getElementsByTagName('pageType')[0].firstChild.nodeValue == 'Example') {
      	full_li_element += ' class="example"';
      	exampleCount++;
      }
			/* adding the link element*/
			full_li_element += '><a href="'+qturl;
			/* adding the URL attribute*/
			full_li_element += propertyTags[i].getElementsByTagName('pageUrl')[j].firstChild.nodeValue;
      		/* adding the link title and closing the link and list elements */
			full_li_element += '">' + propertyTags[i].getElementsByTagName('pageWords')[0].firstChild.nodeValue + '</a></li>';
			/* appending the list element to the #resultlist div*/
			$('#resultlist').append(full_li_element);
		}
	}

	/* if the result is not empty */
	if (propertyTags.length > 0) {
	/* add class name active to show the dialog */
	  $('#resultdialog').addClass('active');
	  /* setting number of hits*/
	  $('#resultcount').html(propertyTags.length);
	  $('#apicount').html(apiCount);
	  $('#articlecount').html(articleCount);
	  $('#examplecount').html(exampleCount);
	  
	}
	else {
	  $('#pageType').addClass('red');
	  }
  


  // Filtering results in display
	$('p#resultlinks a').click(function(e) {
  	e.preventDefault();
	// Displays API ref pages
		if (this.id == "showapiresults") {
			$("#resultlist").removeClass().addClass('api');
			$("#resultlinks").removeClass().addClass('api');
			$("#searchcount").removeClass().addClass('api');
		}
	// Displays Articles
		else if (this.id == "showarticleresults") {
			$("#resultlist").removeClass().addClass('article');
			$("#resultlinks").removeClass().addClass('article');
			$("#searchcount").removeClass().addClass('article');
		}
	// Displays Examples
		if (this.id == "showexampleresults") {
			$("#resultlist").removeClass().addClass('example');
			$("#resultlinks").removeClass().addClass('example');
			$("#searchcount").removeClass().addClass('example');
		}
	// Displays All
		if (this.id == "showallresults") {
			$("#resultlist").removeClass().addClass('all');
			$("#resultlinks").removeClass().addClass('all');
			$("#searchcount").removeClass().addClass('all');
		}
	});
}

//build regular expression object to find empty string or any number of blank
var blankRE=/^\s*$/;


function CheckEmptyAndLoadList()
{
	/* extracts search query */
	var value = document.getElementById('pageType').value; 
	/* if the search is less than three chars long remove class names and remove elements from old search*/
	if((blankRE.test(value)) || (value.length < 3))
	{
	$('#resultdialog').removeClass('active');
	$('#resultlist li').remove();
	}
}

// Loads on doc ready - prepares search 
	$(document).ready(function () {
	/* fetch page title*/ 
	var pageTitle = $('title').html();
	/* getting content from search box */
          var currentString = $('#pageType').val() ;
	  /* if the search box is not empty run CheckEmptyAndLoadList*/
		  if(currentString.length < 1){
      	   		CheckEmptyAndLoadList();			
		  }

		/* on key-up in the search box execute the following */
        $('#pageType').keyup(function () {
		/* extract the search box content */
          var searchString = $('#pageType').val() ;
	  /* if the string is less than three characters */
          if ((searchString == null) || (searchString.length < 3)) {
			/* remove classes and elements*/
				$('#pageType').removeClass('loading');
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
				$('#pageType').addClass('loading');
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
				$('#pageType').removeClass('loading');

                processNokiaData(response);

 }     
              });
            }, 500); /* timer set to 500 ms */
        });
      }); 
