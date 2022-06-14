/*Project section dropdown*/
$( function() {
    $( ".project_item" ).accordion({
      collapsible: true,
      heightStyle: "content",
      active: false,
      icons: false,
      beforeActivate: function(){
        $(".project_item").not(this).accordion('option', 'active', false); //If some item is opened close it before opening next one
      }
    });
});


/*Navigation scrolling*/
window.addEventListener('scroll', () => nav_move(this.scrollY)); 

function nav_move(height){
  var header = document.getElementById('header');
  var nav_pc = document.getElementById('nav_pc');
  if(header.offsetHeight < height && window.getComputedStyle(nav_pc).display !== "none"){
    // If navigation is no longer in screen view and is visible (not visible in mobile form), change header class to sticky
    header.className = "sticky";
  }
  else{
    //Else remove the class
    header.classList.remove("sticky");
  }
}


/*Hamburger menu*/
hamb_icon = document.getElementById('hamb_icon');
hamb_icon.addEventListener('click', () => show_hamurger()); //If there was a click on the menu icon call show_hamburger()

function show_hamurger(){
  links = document.getElementById('mobile_links');
  subsections = document.getElementsByClassName('dropdown_mobile active');

  if(links.className == "hamb_active" || links.className == "hamb_active subsection"){
    //if menu is opened and there was a click close the menu
    links.classList.remove("hamb_active");
    links.classList.remove("subsection");

    for (var i = 0; i < subsections.length; i++){
      //Close all subsections
      subsections.item(i).className = "dropdown_mobile";
    }

  }
  else{
    //Else open the menu (change its class)
    links.className = "hamb_active";
    dropdown_icons = document.getElementsByClassName('dropdown_icon');
    var drop_click = false;

    links.addEventListener('click', () =>{
      //If a click occured on some content of hamburger menu
      for (var i = 0; i < dropdown_icons.length; i++){
        if(dropdown_icons.item(i) === event.target){
          //If the click was targeted at subsection dropdown
          drop_click = true;
        }
      }
      if(drop_click == false){
        //If the click wasnt targeted at subsection dropdown close the menu and subsections
        links.classList.remove("hamb_active");
        links.classList.remove("subsection");

        for (var i = 0; i < subsections.length; i++){
          subsections.item(i).className = "dropdown_mobile";
        }
      }
      drop_click = false;
    })
  }
}


/*Navigation Subsection dropdown*/
dropdown_icons = document.getElementsByClassName('dropdown_icon');
links = document.getElementById('mobile_links');

for (var i = 0; i < dropdown_icons.length; i++) {
  dropdown_icons.item(i).addEventListener('click', () => {
    //Check for click on dropdown symbol

    parent_node = event.target.parentNode.parentNode; //Get the parent div

    if(parent_node.className === "dropdown_mobile active"){
      //If it is already opened, close it
      parent_node.className = "dropdown_mobile";
      links.className = "hamb_active";
    }
    else{
      //Else open it
      parent_node.className = "dropdown_mobile active";
      links.className = "hamb_active subsection";
    }
  });
};