(function(_ds){var window=this;var p6=function(a){_ds.Sj(a);if("indeterminate"===a.getAttribute("type")){const b=a.getAttribute("color"),c=document.createElement("div");c.classList.add("devsite-progress--indeterminate");for(let d=1;4>=d;d++){const e=document.createElement("div");e.classList.add(`devsite-progress--indeterminate-${d}`);!b||1!==d&&3!==d||(e.style.backgroundColor=b);c.appendChild(e)}a.appendChild(c)}},q6=class extends _ds.F{static get observedAttributes(){return["color","type"]}connectedCallback(){p6(this)}attributeChangedCallback(a,
b,c){b!==c&&p6(this)}};q6.prototype.attributeChangedCallback=q6.prototype.attributeChangedCallback;q6.prototype.connectedCallback=q6.prototype.connectedCallback;try{customElements.define("devsite-progress",q6)}catch(a){console.warn("devsite.app.customElement.DevsiteProgress",a)};})(_ds_www);
