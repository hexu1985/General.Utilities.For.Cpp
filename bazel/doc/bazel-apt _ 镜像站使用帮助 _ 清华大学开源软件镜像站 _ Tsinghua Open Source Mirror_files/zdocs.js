"use strict";window.addEventListener("DOMContentLoaded",function(){function s(e){var t=Object.fromEntries(new FormData(e).entries());Array.from(e.querySelectorAll("input[type=checkbox]:not(:checked)")).forEach(function(e){t[e.name]=""});var n={};for(var r in t){n[r]=t[r];var o=u.input[r];if(o){var a=null;"option"in o?a=o.option[t[r]]:("true"in o||"false"in o)&&(a="on"===t[r]?o["true"]:o["false"]),"object"==typeof a&&Object.assign(n,a),"string"==typeof a&&(n[r]=a)}}return n}function t(e){var t={path:(window.mirrorId.endsWith(".git")?"/git/":"/")+window.mirrorId};Array.from(document.querySelectorAll("form.z-global")).forEach(function(e){Object.assign(t,s(e))}),t.scheme=t._scheme?"https":"http",t.host=t.host.replace(/^https?:\/\//,""),t.sudo=t._sudo?"sudo ":"",t.filter&&u.filter.scheme&&(t.scheme=u.filter.scheme);var n=e.previousElementSibling,r=n.querySelector("form.z-form"),o=n.querySelector("pre.z-code");null===o&&((o=document.createElement("pre")).classList.add("z-code"),n.appendChild(o)),r&&Object.assign(t,s(r)),t.endpoint=t.scheme+"://"+t.host+t.path;var a=globalThis.Mustache.render(e.textContent.trim(),t,{},{escape:function i(e){return e}});try{var l=e.attributes.getNamedItem("z-lang");l&&hljs.getLanguage(l.value)&&(a=hljs.highlight(a,{language:l.value}).value)}catch(c){console.error(c)}o.innerHTML=a}function e(e){!e||e.currentTarget.classList.contains("z-global")?Array.from(document.querySelectorAll(".z-help pre.z-tmpl")).forEach(t):t(e.currentTarget.parentElement.nextElementSibling)}var u=JSON.parse(atob(document.getElementById("z-config").textContent));u.filter&&u.filter.scheme&&(document.querySelector('input[name="_scheme"]').parentElement.style.display="none"),e(null);var n=function n(e){return e.preventDefault()},r=!0,o=!1,a=undefined;try{for(var l,i=document.querySelectorAll("form.z-form")[Symbol.iterator]();!(r=(l=i.next()).done);r=!0){var c=l.value;c.addEventListener("submit",n),c.classList.contains("z-global")?c.addEventListener("change",function(){return e(null)}):c.addEventListener("change",e)}}catch(d){o=!0,a=d}finally{try{!r&&i["return"]&&i["return"]()}finally{if(o)throw a}}});